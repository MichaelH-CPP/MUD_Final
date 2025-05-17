#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <mosquitto.h>
#include <stdatomic.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <syslog.h>

// Define Mosquitto Variables
#define PORT 1883
#define KEEPALIVE 15
#define TOPIC_SUB "mudClient"
#define TOPIC_PUB "espRequest"
struct mosquitto *mosq = NULL;
char mqtt_server[20];

// Define Global Variables
#define MAX_STR_LEN 100
char *maps[4] = {"mapA", "mapD", "mapM", "mapT"};
char ***map = NULL;
int rows = 0, cols = 0;
int currRow = 0, currCol = 0;

// Define Input Variables (for callbacks)
char nextString[MAX_STR_LEN];
atomic_bool input = ATOMIC_VAR_INIT(false);

// Add these daemon-specific defines
#define PID_FILE "/var/run/mud_daemon.pid"
#define LOG_IDENT "mud_daemon"
volatile sig_atomic_t running = 1;

/*
    [Daemon Functions]
*/
void daemonize()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        syslog(LOG_ERR, "Failed to fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
        exit(EXIT_SUCCESS); // Parent exits

    if (setsid() < 0)
    {
        syslog(LOG_ERR, "Failed to create session");
        exit(EXIT_FAILURE);
    }

    umask(0);
    chdir("/");

    // Close standard descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
void cleanup()
{
    remove(PID_FILE);
    closelog();
}
void signal_handler(int sig)
{
    switch (sig)
    {
    case SIGTERM:
        syslog(LOG_INFO, "Received shutdown signal");
        running = 0; // Signal main loop to exit
        break;
    case SIGHUP:
        syslog(LOG_INFO, "Reloading configuration");
        // Add reload logic if needed
        break;
    }
}

/*
    [Mosquitto Functions]
*/
void publish_response(const char *message)
{
    mosquitto_publish(mosq, NULL, TOPIC_PUB, strlen(message) + 1, message, 0, false);
}
void callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
    (void)mosq;
    (void)userdata;

    nextString[0] = '\0';

    if (!msg || !msg->payload)
    {
        input = true;
        return;
    }

    size_t max_copy = MAX_STR_LEN - 1;
    size_t len = (size_t)msg->payloadlen < max_copy ? (size_t)msg->payloadlen : max_copy;

    memcpy(nextString, (char *)msg->payload, len);
    nextString[len] = '\0';

    atomic_store(&input, true);
}
void waitForInput()
{
    fflush(stdout); // Force flush

    int rc;
    do
    {
        rc = mosquitto_loop(mosq, 1000, 1); // Timeout after 1 second
    } while (!atomic_load(&input) && rc == MOSQ_ERR_SUCCESS);

    atomic_store(&input, false);
}

/*
  [Map Functions]
  void set_map():
    - Remakes the map array
    - Is called only when guaranteed to make a new map

  void free_map(bool delete):
    - clears all data from the map
    - if delete is true, will completely delete the map array

  void shuffle_maps(char* array)
*/
void free_map(bool delete)
{
    if (map != NULL)
    {
        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < cols; c++)
            {
                free(map[r][c]);
            }
            free(map[r]);
        }
    }
    if (delete)
    {
        free(map);
        map = NULL;
        rows = cols = 0;
    }
}
void set_map(char command[MAX_STR_LEN])
{
    char full_command[256];
    snprintf(full_command, sizeof(full_command), "./%s.sh %s", command, mqtt_server);
    system(full_command);

    free_map(false);
    waitForInput();
    rows = atoi(nextString);
    input = false;

    waitForInput();
    cols = atoi(nextString);
    input = false;

    // Allocate new space
    map = malloc(rows * sizeof(char *));
    for (int r = 0; r < rows; r++)
    {
        map[r] = malloc(cols * sizeof(char *));
        for (int c = 0; c < cols; c++)
        {
            waitForInput();
            map[r][c] = strdup(nextString);
        }
    }
}
void shuffle_maps(char **arr)
{
    for (int i = 3; i > 0; i--)
    {
        int j = rand() % (i + 1);
        // Swap pointers, not string data
        char *temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

/*
  [Game Functions]
  void game():
    - Handles all the logic of the game

  void move():
    - Runs during game
    - Checks to make sure if player can move forward or not
    - Calls set_map when it needs to change the map
*/
void move(int *currMap)
{
    waitForInput();
    // West
    if (strcmp(nextString, "east") == 0)
    {
        if ((currCol + 1) == cols)
        {
            if (currRow != 0 || (*currMap + 1) >= 4)
            {
                publish_response("Wall in the way, cannot go East");
            }
            else
            {
                set_map(maps[++*currMap]);
                currRow = currCol = 0;
            }
        }
        else if (tolower(map[currRow][currCol + 1][0]) != 'w')
        {
            currCol++;
        }
        else
        {
            publish_response(map[currRow][currCol + 1] + 2);
        }
    }

    // East
    else if (strcmp(nextString, "west") == 0)
    {
        if (currCol == 0)
        {
            if (currRow != 0 || (*currMap - 1) < 0)
            {
                publish_response("Wall in the way, cannot go West");
            }
            else
            {
                set_map(maps[--*currMap]);
                currRow = 0;
                currCol = cols - 1;
            }
        }
        else if (tolower(map[currRow][currCol - 1][0]) != 'W')
        {
            currCol--;
        }
        else
        {
            publish_response(map[currRow][currCol - 1] + 2);
        }
    }

    // North
    else if (strcmp(nextString, "north") == 0)
    {
        if (currRow == 0 || tolower(map[currRow - 1][currCol][0]) == 'w')
        {
            publish_response(map[currRow - 1][currCol] + 2);
        }
        else
        {
            currRow--;
        }
    }

    // South
    else if (strcmp(nextString, "south") == 0)
    {
        if ((currRow + 1) == rows || tolower(map[currRow + 1][currCol][0]) == 'w')
        {
            publish_response(map[currRow + 1][currCol] + 2);
        }
        else
        {
            currRow++;
        }
    }
    usleep(1500 * 1000);
}
void game()
{
    shuffle_maps(maps);
    int currMap = rand() % 4;
    set_map(maps[currMap]);

    while (true)
    {

        char space[MAX_STR_LEN];
        strcpy(space, map[currRow][currCol]);
        if (tolower(space[0]) == 'i')
        {
            break;
        }
        else if (strlen(space) >= 2)
        {
            publish_response(space + 2);
            move(&currMap);
        }
    }
    if (strlen(map[currRow][currCol]) >= 2)
    {
        publish_response(map[currRow][currCol] + 2);
    }
    else
    {
        publish_response(map[currRow][currCol]);
    }

    free_map(true);
}
int main(int argc, char *argv[])
{
    daemonize();

    // PID file handling
    FILE *pidfile = fopen(PID_FILE, "w");
    if (!pidfile)
    {
        syslog(LOG_ERR, "Could not create PID file: %m");
        exit(EXIT_FAILURE);
    }
    fprintf(pidfile, "%d\n", getpid());
    fclose(pidfile);

    // Signal handling
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);

    // Syslog setup
    openlog(LOG_IDENT, LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon started");

    // Your original main() code with printf replaced by syslog
    if (argc < 2)
    {
        syslog(LOG_ERR, "Usage: %s <mqtt-server>", argv[0]);
        exit(EXIT_FAILURE);
    }

    strcpy(mqtt_server, argv[1]);
    mosquitto_lib_init();

    // ... rest of your original main() code ...
    mosq = mosquitto_new(NULL, true, NULL);

    if (!mosq)
    {
        syslog(LOG_ERR, "Failed to create Mosquitto instance\n");
        return 1;
    }

    mosquitto_message_callback_set(mosq, callback);
    int rc = mosquitto_connect(mosq, mqtt_server, PORT, KEEPALIVE);
    if (rc != 0)
    {
        syslog(LOG_ERR, "Unable to connect to broker. Error Code: %d\n", rc);
        return 1;
    }

    if (getuid() == 0)
    {
        if (setgid(994) != 0 || setuid(999) != 0)
        {
            syslog(LOG_ERR, "Failed to drop privileges");
            cleanup();
            exit(EXIT_FAILURE);
        }
        syslog(LOG_INFO, "Dropped privileges to UID %d/GID %d", 999, 994);
    }

    mosquitto_subscribe(mosq, NULL, TOPIC_SUB, 0);

    publish_response("connected");
    srand(time(NULL));

    // Display Prompt then Start Game
    char menu[MAX_STR_LEN] = "Welcome! Press any button to start.";
    publish_response(menu);
    waitForInput();

    // Daemon main loop
    syslog(LOG_INFO, "Running as UID: %d, GID: %d", getuid(), getgid());
    while (running)
    {
        game();
        sleep(1);
    }
    // Cleanup after loop exits
    syslog(LOG_INFO, "Performing final cleanup");
    cleanup();
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    remove(PID_FILE);
    return 0;
}