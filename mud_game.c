#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <mosquitto.h>

// Define Mosquitto Variables
#define BROKER "34.94.82.232"
#define PORT 1883
#define KEEPALIVE 15
#define TOPIC_SUB "mudClient"
#define TOPIC_PUB "espClient"
struct mosquitto *mosq = NULL;

// Define Global Variables
#define MAX_STR_LEN 100
char maps[4][MAX_STR_LEN] = {"./mapA.sh", "./mapD.sh", "./mapM.sh", "./mapT.sh"};
bool input = false;
char **map = NULL;
char nextString[MAX_STR_LEN];
int rows = 0, cols = 0;
int currRow = 0, currCol = 0;
char move[10];

/*
    [Mosquitto Functions]
*/
void publish_response(const char *message)
{
    mosquitto_publish(mosq, NULL, TOPIC_PUB, strlen(message) + 1, message, 0, false);
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
void set_map(char command[MAX_STR_LEN])
{
    system(command);
    while (!input)
    {
    }
    rows = atoi(nextString);
    input = false;

    free_map(false);
    while (!input)
    {
    }
    cols = atoi(nextString);
    input = false;

    // Allocate new space
    map = malloc(rows * sizeof(char *));
    for (int i = 0; i < rows; i++)
    {
        map[i] = malloc(cols * MAX_STR_LEN);
        memset(map[i], 0, cols * MAX_STR_LEN); // Initialize to zeros
    }

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            while (!input)
            {
            }
            map[r][c] = nextString;
            input = false;
        }
    }
}
void free_map(bool delete)
{
    if (map != NULL)
    {
        for (int i = 0; i < rows; i++)
        {
            free(map[i]);
        }
    }
    if (delete)
    {
        free(map);
        map = NULL;
        rows = cols = 0;
    }
}
void shuffle_maps(char arr[][20])
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
void callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
    printf("Received on [%s]: %.*s\n", msg->topic, msg->payloadlen, (char *)msg->payload);
}

void game()
{
    shuffle_maps(maps);
    int currMap = rand() % 4;
    set_map(maps[currMap]);

    while (true)
    {
        publish_response(map[currRow][currCol]);
    }
}

void setup()
{
    // Connect to WiFi and Server
    srand(time(NULL));
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);

    if (!mosq)
    {
        fprintf(stderr, "Failed to create Mosquitto instance\n");
        return 1;
    }

    mosquitto_message_callback_set(mosq, callback);
    int rc = mosquitto_connect(mosq, BROKER, PORT, KEEPALIVE);
    if (rc != 0)
    {
        fprintf(stderr, "Unable to connect to broker. Error Code: %d\n", rc);
        return 1;
    }

    mosquitto_subscribe(mosq, NULL, TOPIC_SUB, 0);

    publish_response("connected");
    srand(time(NULL));
}

int main()
{
    setup();
    char menu[MAX_STR_LEN] = "Welcome! Press any button to start.";
    publish_response(menu);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}