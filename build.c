#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the structure for the circular doubly linked list node
typedef struct SongNode {
    char title[256];
    int playCount; // Add play count for each song
    struct SongNode* prev;
    struct SongNode* next;
} SongNode;

// Function to create a new node
SongNode* createNode(const char* title) {
    SongNode* newNode = (SongNode*)malloc(sizeof(SongNode));
    if (newNode != NULL) {
        strncpy(newNode->title, title, sizeof(newNode->title));
        newNode->playCount = 0;
        newNode->prev = newNode;
        newNode->next = newNode;
    }
    return newNode;
}

// Function to insert a new song node in descending order of play count
void insertSong(SongNode** head, const char* title, int playCount) {
    SongNode* newNode = createNode(title);
    if (newNode == NULL) {
        return;
    }

    newNode->playCount = playCount; // Set the play count

    if (*head == NULL || newNode->playCount > (*head)->playCount) {
        // Insert at the beginning if the list is empty or new node has higher play count
        if (*head != NULL) {
            newNode->next = *head;
            newNode->prev = (*head)->prev;
            (*head)->prev->next = newNode;
            (*head)->prev = newNode;
        } else {
            newNode->next = newNode;
            newNode->prev = newNode;
        }

        *head = newNode;
    } else {
        // Traverse the list to find the correct position based on play count (descending order)
        SongNode* current = *head;
        while (current->next != *head && current->next->playCount >= newNode->playCount) {
            current = current->next;
        }

        // Insert the new node
        newNode->next = current->next;
        newNode->prev = current;
        current->next = newNode;
        newNode->next->prev = newNode;
    }
}

// Function to display instructions to the user
void displayInstructions() {
    DrawText("Instructions:", 10, 10, 20, DARKGRAY);
    DrawText("Press 'N' for Next Song", 90, 40, 20, DARKGRAY);
    DrawText("Press 'Space' to Pause the Song", 90, 70, 20, DARKGRAY);
    DrawText("Press 'P' for Previous Song", 90, 100, 20, DARKGRAY);
    DrawText("Press 'Q' to Quit", 90, 130, 20, DARKGRAY);
}

// Function to save the song list to a text file
void saveSongsToTextFile(SongNode* head) {
    FILE* file = fopen("user_input.txt", "w");
    if (file == NULL) {
        printf("Failed to open the file for writing.\n");
        return;
    }

    SongNode* current = head;
    do {
        fprintf(file, "%s,%d\n", current->title, current->playCount);
        current = current->next;
    } while (current != head); // Traverse the entire circular list

    fclose(file);
}

// Function to load the song list from a text file
SongNode* loadSongsFromTextFile() {
    SongNode* playlist = NULL;
    FILE* file = fopen("user_input.txt", "r");
    if (file == NULL) {
        printf("Failed to open the file for reading.\n");
        return NULL;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        char title[256];
        int playCount;
        if (sscanf(line, "%[^,],%d\n", title, &playCount) == 2) {
            insertSong(&playlist, title, playCount);
        }
    }

    fclose(file);
    return playlist;
}

// Function to play the songs in the circular doubly linked list

void playSongs(SongNode* head) {
    InitAudioDevice(); // Initialize audio device

    if (head == NULL) {
        printf("The playlist is empty \n");
         CloseAudioDevice();  
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Playlist is empty add songs:", 100, 10, 20, DARKGRAY);
            EndDrawing();
            
        }
     
        return;
    }

    SongNode* current = head;
    Music music = { 0 };
    bool isPlaying = false;
    bool isPaused = false; // New variable to track pause state

    while (!WindowShouldClose()) {
        displayInstructions(); // Display instructions to the user

        if (!isPlaying && !isPaused) {
            printf("Now playing: %s (Play Count: %d)\n", current->title, current->playCount);

            // Load and play the song using Raylib functions
            music = LoadMusicStream(current->title);
            PlayMusicStream(music);
            isPlaying = true;

            // Increment the play count for the current song
            current->playCount++;
        }

        UpdateMusicStream(music);
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw the current song title
        DrawText(current->title, GetScreenWidth() / 2 - MeasureText(current->title, 30) / 2, GetScreenHeight() / 2 - 15, 30, DARKGRAY);

        // Draw play count
        DrawText(TextFormat("Play Count: %d", current->playCount), GetScreenWidth() / 2 - 70, GetScreenHeight() / 2 + 20, 20, DARKGRAY);

        EndDrawing();

        // Check for 'N' key press to play the next song
        if (IsKeyPressed(KEY_N)) {
            if (isPlaying) {
                StopMusicStream(music);
                UnloadMusicStream(music);
                isPlaying = false;
                isPaused = false;
            }
            current = current->next;
        }

        // Check for 'P' key press to play the previous song
        if (IsKeyPressed(KEY_P)) {
            if (isPlaying) {
                StopMusicStream(music);
                UnloadMusicStream(music);
                isPlaying = false;
                isPaused = false;
            }
            current = current->prev;
        }

        // Check for 'Q' key press to quit
        if (IsKeyPressed(KEY_Q)) {
            break;
        }

        // Check for 'Space' key press to pause/unpause
        if (IsKeyPressed(KEY_SPACE)) {
            if (isPlaying && !isPaused) {
                PauseMusicStream(music);
                isPaused = true;
            } else if (isPlaying && isPaused) {
                ResumeMusicStream(music);
                isPaused = false;
            }
        }
    }

    // Save the updated song list to the text file before exiting
    saveSongsToTextFile(head);

    CloseAudioDevice(); // Close audio device
}


int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Music Player");

    char userInput[256] = {0};
    bool saveButtonClicked = false;
    bool playButtonClicked = false;

    SongNode* songList = loadSongsFromTextFile(); // Load songs from the text file

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        Rectangle inputBox = {screenWidth / 2 - 200, 10, 400, 40};
        DrawRectangleLines(inputBox.x, inputBox.y, inputBox.width, inputBox.height, DARKGRAY);

        DrawText(userInput, inputBox.x + 10, inputBox.y + 10, 20, DARKGRAY);
        DrawText("Save to Text File and Play:", inputBox.x + inputBox.width + 20, 10, 20, DARKGRAY);

        Rectangle saveButton = {inputBox.x + inputBox.width + 20, 40, 100, 40};
        Rectangle playButton = {screenWidth / 2 - 75, 90, 150, 50};

        if (CheckCollisionPointRec(GetMousePosition(), saveButton)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                saveButtonClicked = true;
                playButtonClicked = false;

                FILE* file = fopen("user_input.txt", "a");
                if (file) {
                    fprintf(file, "%s\n", userInput);
                    fclose(file);
                }

                insertSong(&songList, userInput, 0);
            }
        } else {
            saveButtonClicked = false;
        }

        if (CheckCollisionPointRec(GetMousePosition(), playButton)) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                playButtonClicked = true;
                saveButtonClicked = false;
                playSongs(songList);
            }
        } else {
            playButtonClicked = false;
        }

        DrawRectangleRec(saveButton, saveButtonClicked ? GREEN : DARKGRAY);
        DrawText("Save", saveButton.x + 20, saveButton.y + 10, 20, WHITE);

        DrawRectangleRec(playButton, playButtonClicked ? GREEN : DARKGRAY);
        DrawText("Play Songs", playButton.x + 35, playButton.y + 10, 20, WHITE);

        int key = GetCharPressed();
        if (key > 0 && strlen(userInput) < sizeof(userInput) - 1) {
            if ((key >= 32 && key <= 125) || key == ',') {
                userInput[strlen(userInput)] = (char)key;
            }
        }

        if (IsKeyPressed(KEY_BACKSPACE) && strlen(userInput) > 0) {
            userInput[strlen(userInput) - 1] = '\0';
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
    