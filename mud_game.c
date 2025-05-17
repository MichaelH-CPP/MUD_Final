#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <mosquitto.h>
#include <stdatomic.h>

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

/*
    [Mosquitto Functions]
*/
void publish_response(const char *message)
{
    mosquitto_publish(mosq, NULL, TOPIC_PUB, strlen(message) + 1, message, 0, false);
}

void callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
    // Always initialize first byte
    nextString[0] = '\0';

    // 1. Handle NULL payload case
    if (!msg || !msg->payload)
    {
        input = true; // Still trigger processing
        return;
    }

    // 2. Simplified safe copy
    size_t max_copy = MAX_STR_LEN - 1;
    size_t len = msg->payloadlen < max_copy ? msg->payloadlen : max_copy;

    memcpy(nextString, (char *)msg->payload, len); // No strncpy quirks
    nextString[len] = '\0';                        // Direct null-termination

    // printf("Received: [%s] '%s'\n", msg->topic, nextString);
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
}
int main(int argc, char *argv[])
{
    strcpy(mqtt_server, argv[1]);
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);

    if (!mosq)
    {
        fprintf(stderr, "Failed to create Mosquitto instance\n");
        return 1;
    }

    mosquitto_message_callback_set(mosq, callback);
    int rc = mosquitto_connect(mosq, mqtt_server, PORT, KEEPALIVE);
    if (rc != 0)
    {
        fprintf(stderr, "Unable to connect to broker. Error Code: %d\n", rc);
        return 1;
    }

    mosquitto_subscribe(mosq, NULL, TOPIC_SUB, 0);

    publish_response("connected");
    srand(time(NULL));

    // Display Prompt then Start Game
    char menu[MAX_STR_LEN] = "Welcome! Press any button to start.";
    publish_response(menu);
    waitForInput();
    game();

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}