#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <mosquitto.h>

// Define Mosquitto Variables
#define PORT 1883
#define KEEPALIVE 15
#define TOPIC_SUB "mudClient"
#define TOPIC_PUB "espRequest"
struct mosquitto *mosq = NULL;
char mqtt_server[20];

// Define Global Variables
#define MAX_STR_LEN 100
char *maps[4] = {"mapA", "mapA", "mapM", "mapT"};
char ***map = NULL;
int rows = 0, cols = 0;
int currRow = 0, currCol = 0;

// Define Input Variables (for callbacks)
char nextString[MAX_STR_LEN];
bool input = false;

/*
    [Mosquitto Functions]
*/
void publish_response(const char *message)
{
    mosquitto_publish(mosq, NULL, TOPIC_PUB, strlen(message) + 1, message, 0, false);
}
void callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
    printf("Received on [%s]: %.*s\n", msg->topic, msg->payloadlen, (char *)msg->payload);
    nextString[0] = '\0';
    size_t len = msg->payloadlen < MAX_STR_LEN - 1 ? msg->payloadlen : MAX_STR_LEN - 1;
    strncpy(nextString, (char *)msg->payload, len);
    nextString[MAX_STR_LEN - 1] = '\0'; // Explicit null-termination
}
void waitForInput()
{
    int rc = mosquitto_loop(mosq, -1, 1);
    while (!input && rc == MOSQ_ERR_SUCCESS)
    {
        rc = mosquitto_loop(mosq, -1, 1);
    }
    input = false;
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
    printf("set_map function");
    snprintf(full_command, sizeof(full_command), "./%s.sh %s", command, mqtt_server);
    printf(full_command);
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
        char temp[20];
        strcpy(temp, arr[i]);
        strcpy(arr[i], arr[j]);
        strcpy(arr[j], temp);
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
    if (strcmp(nextString, "west") == 0)
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
            publish_response("Wall in the way, cannot go West.");
        }
    }

    // East
    else if (strcmp(nextString, "east") == 0)
    {
        if (currCol == 0)
        {
            if (currRow != 0 || (*currMap - 1) < 0)
            {
                publish_response("Wall in the way, cannot go East");
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
            publish_response("Wall in the way, cannot go East.");
        }
    }

    // North
    else if (strcmp(nextString, "north") == 0)
    {
        if (currRow == 0 || tolower(map[currRow - 1][currCol][0]) == 'w')
        {
            publish_response("Wall in the way, cannot go North");
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
            publish_response("Wall in the way, cannot go South");
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

    // Display Prompt then Start Game
    char menu[MAX_STR_LEN] = "Welcome! Press any button to start.";
    publish_response(menu);
    waitForInput();
    game();

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}