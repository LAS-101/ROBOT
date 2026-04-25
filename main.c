#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "raylib.h"

// Command type definitions
#define TYPE_AVANCER 1
#define TYPE_RECULER 2
#define TYPE_GAUCHE  3
#define TYPE_DROITE  4
#define TYPE_ARRET   5
#define TYPE_RECHARGE 6

// Priority definitions
#define PRIO_NORMALE 0
#define PRIO_URGENTE 1

// Window constants
#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define GRID_SIZE 40

// Robot constants
#define ROBOT_SIZE 30
#define MOVE_SPEED 100.0f
#define ROTATION_SPEED 90.0f

// Command structure
typedef struct commande {
    int id;
    int type;
    int duree;
    int vitesse;
    int priorite;
} Commande;

// Queue element structure
typedef struct element_file {
    Commande donnee;
    struct element_file *next;
} Element;

// Queue structure
typedef struct File {
    Element *head;
    Element *tail;
    int nb_elements;
} File;

// Robot structure
typedef struct Robot {
    float x;
    float y;
    float angle;
    float speed;
    Color color;
    int is_moving;
    int is_recharging;
} Robot;

// Obstacle structure
typedef struct Obstacle {
    float x;
    float y;
    int active;
    float timer;
} Obstacle;

// Global variables
int batterie = 100;
Commande current_command = {0};
float command_timer = 0.0f;
int executing_command = 0;

// Queue functions
File* initialiser_file() {
    File *f = malloc(sizeof(File));
    if (f != NULL) {
        f->head = NULL;
        f->tail = NULL;
        f->nb_elements = 0;
    }
    return f;
}

int est_vide(File *f) {
    return (f == NULL || f->head == NULL);
}

void ajouter_commande_normale(File *f_normal, Commande c) {
    Element *nouvel_elem = malloc(sizeof(Element));
    if (nouvel_elem == NULL) return;
    
    nouvel_elem->donnee = c;
    nouvel_elem->donnee.priorite = PRIO_NORMALE;
    nouvel_elem->next = NULL;
    
    if (est_vide(f_normal)) {
        f_normal->head = nouvel_elem;
    } else {
        f_normal->tail->next = nouvel_elem;
    }
    f_normal->tail = nouvel_elem;
    f_normal->nb_elements++;
}

void ajouter_commande_urgente(File *f_urgent, Commande c) {
    Element *nouvel_elem = malloc(sizeof(Element));
    if (nouvel_elem == NULL) return;
    
    nouvel_elem->donnee = c;
    nouvel_elem->donnee.priorite = PRIO_URGENTE;
    nouvel_elem->next = NULL;
    
    if (est_vide(f_urgent)) {
        f_urgent->head = nouvel_elem;
    } else {
        f_urgent->tail->next = nouvel_elem;
    }
    f_urgent->tail = nouvel_elem;
    f_urgent->nb_elements++;
    
    printf("\n[ALERT] Commande URGENTE ajoutee a la file d'urgence !\n");
}

Commande prendre_prochaine_commande(File *f) {
    Commande c_vide = {0};
    
    if (est_vide(f)) return c_vide;

    Element *temp = f->head;
    Commande c_retour = temp->donnee;

    f->head = temp->next;
    if (f->head == NULL) {
        f->tail = NULL;
    }
    
    free(temp);
    f->nb_elements--;
    return c_retour;
}

void ecrire_journal(Commande c) {
    FILE *fichier = fopen("journal_robot.txt", "a");
    if (fichier == NULL) return;

    time_t now;
    time(&now);
    char *date_str = ctime(&now);
    date_str[strlen(date_str) - 1] = '\0';

    char *type_str;
    switch(c.type) {
        case TYPE_AVANCER: type_str = "AVANCER"; break;
        case TYPE_RECULER: type_str = "RECULER"; break;
        case TYPE_GAUCHE:  type_str = "GAUCHE"; break;
        case TYPE_DROITE:  type_str = "DROITE"; break;
        case TYPE_ARRET:   type_str = "ARRET_URGENCE"; break;
        case TYPE_RECHARGE: type_str = "RECHARGE"; break;
        default: type_str = "INCONNU";
    }

    fprintf(fichier, "[%s] [%s] Action: %s | Duree: %ds\n", 
            date_str, 
            (c.priorite == PRIO_URGENTE ? "URGENT" : "NORMAL"), 
            type_str, c.duree);

    fclose(fichier);
}

// Drawing functions
void draw_grid() {
    for (int i = 0; i < SCREEN_WIDTH; i += GRID_SIZE) {
        DrawLine(i, 0, i, SCREEN_HEIGHT, (Color){40, 40, 40, 255});
    }
    for (int j = 0; j < SCREEN_HEIGHT; j += GRID_SIZE) {
        DrawLine(0, j, SCREEN_WIDTH, j, (Color){40, 40, 40, 255});
    }
}

void draw_robot(Robot *robot) {
    // Calculate robot triangle points
    float rad = robot->angle * DEG2RAD;
    
    // Front point
    Vector2 p1 = {
        robot->x + cosf(rad) * ROBOT_SIZE,
        robot->y + sinf(rad) * ROBOT_SIZE
    };
    
    // Back left point
    Vector2 p2 = {
        robot->x + cosf(rad + 2.5f) * ROBOT_SIZE * 0.7f,
        robot->y + sinf(rad + 2.5f) * ROBOT_SIZE * 0.7f
    };
    
    // Back right point
    Vector2 p3 = {
        robot->x + cosf(rad - 2.5f) * ROBOT_SIZE * 0.7f,
        robot->y + sinf(rad - 2.5f) * ROBOT_SIZE * 0.7f
    };
    
    // Draw robot body
    DrawTriangle(p1, p2, p3, robot->color);
    DrawTriangleLines(p1, p2, p3, WHITE);
    
    // Draw direction indicator
    Vector2 dir_end = {
        robot->x + cosf(rad) * (ROBOT_SIZE + 15),
        robot->y + sinf(rad) * (ROBOT_SIZE + 15)
    };
    DrawLineEx((Vector2){robot->x, robot->y}, dir_end, 3.0f, YELLOW);
    
    // Draw recharge effect
    if (robot->is_recharging) {
        DrawCircleLines(robot->x, robot->y, ROBOT_SIZE + 10, GREEN);
        DrawCircleLines(robot->x, robot->y, ROBOT_SIZE + 15, GREEN);
    }
}

void draw_obstacle(Obstacle *obs) {
    if (obs->active) {
        DrawCircle(obs->x, obs->y, 25, RED);
        DrawCircleLines(obs->x, obs->y, 25, MAROON);
        DrawText("!", obs->x - 5, obs->y - 10, 30, WHITE);
    }
}

void draw_ui(Robot *robot, File *normal, File *urgent, Commande *cmd, int executing) {
    // Panel background
    DrawRectangle(0, 0, SCREEN_WIDTH, 120, (Color){20, 20, 20, 230});
    
    // Title
    DrawText("ROBOT CONTROL SYSTEM", 20, 10, 30, RAYWHITE);
    
    // Battery
    Color bat_color = batterie > 50 ? GREEN : (batterie > 20 ? ORANGE : RED);
    DrawText(TextFormat("Battery: %d%%", batterie), 20, 50, 20, bat_color);
    DrawRectangle(20, 75, 200, 20, DARKGRAY);
    DrawRectangle(20, 75, (batterie * 2), 20, bat_color);
    
    // Position
    DrawText(TextFormat("Position: (%.0f, %.0f)", robot->x, robot->y), 250, 50, 20, SKYBLUE);
    DrawText(TextFormat("Angle: %.0f°", robot->angle), 250, 75, 20, SKYBLUE);
    
    // Queue info
    DrawText(TextFormat("Normal Queue: %d", normal->nb_elements), 480, 50, 20, BLUE);
    DrawText(TextFormat("Urgent Queue: %d", urgent->nb_elements), 480, 75, 20, RED);
    
    // Current command
    if (executing) {
        char *type_str;
        Color cmd_color;
        
        switch(cmd->type) {
            case TYPE_AVANCER: 
                type_str = "AVANCER"; 
                cmd_color = GREEN;
                break;
            case TYPE_RECULER: 
                type_str = "RECULER"; 
                cmd_color = ORANGE;
                break;
            case TYPE_GAUCHE: 
                type_str = "GAUCHE"; 
                cmd_color = PURPLE;
                break;
            case TYPE_DROITE: 
                type_str = "DROITE"; 
                cmd_color = PURPLE;
                break;
            case TYPE_ARRET: 
                type_str = "ARRET!"; 
                cmd_color = RED;
                break;
            case TYPE_RECHARGE: 
                type_str = "RECHARGE"; 
                cmd_color = YELLOW;
                break;
            default: 
                type_str = "IDLE";
                cmd_color = GRAY;
        }
        
        DrawText(TextFormat("[%s] %s (ID:%d)", 
                 cmd->priorite == PRIO_URGENTE ? "URGENT" : "NORMAL",
                 type_str, cmd->id), 
                 700, 50, 20, cmd_color);
        
        DrawText(TextFormat("Duration: %.1fs", command_timer), 700, 75, 20, LIGHTGRAY);
    } else {
        DrawText("Status: IDLE", 700, 50, 20, GRAY);
    }
}

// Robot update function
void update_robot(Robot *robot, Commande *cmd, float deltaTime) {
    if (!executing_command) return;
    
    float speed_factor = (cmd->vitesse / 100.0f);
    
    switch(cmd->type) {
        case TYPE_AVANCER:
            robot->x += cosf(robot->angle * DEG2RAD) * MOVE_SPEED * speed_factor * deltaTime;
            robot->y += sinf(robot->angle * DEG2RAD) * MOVE_SPEED * speed_factor * deltaTime;
            robot->is_moving = 1;
            break;
            
        case TYPE_RECULER:
            robot->x -= cosf(robot->angle * DEG2RAD) * MOVE_SPEED * speed_factor * deltaTime;
            robot->y -= sinf(robot->angle * DEG2RAD) * MOVE_SPEED * speed_factor * deltaTime;
            robot->is_moving = 1;
            break;
            
        case TYPE_GAUCHE:
            robot->angle -= ROTATION_SPEED * deltaTime;
            robot->is_moving = 1;
            break;
            
        case TYPE_DROITE:
            robot->angle += ROTATION_SPEED * deltaTime;
            robot->is_moving = 1;
            break;
            
        case TYPE_ARRET:
            robot->is_moving = 0;
            robot->color = RED;
            break;
            
        case TYPE_RECHARGE:
            robot->is_recharging = 1;
            robot->is_moving = 0;
            robot->color = YELLOW;
            break;
    }
    
    // Wrap around screen
    if (robot->x < 0) robot->x = SCREEN_WIDTH;
    if (robot->x > SCREEN_WIDTH) robot->x = 0;
    if (robot->y < 120) robot->y = 120;
    if (robot->y > SCREEN_HEIGHT) robot->y = 120;
    
    // Normalize angle
    if (robot->angle < 0) robot->angle += 360;
    if (robot->angle >= 360) robot->angle -= 360;
}

// Sensor verification
void verifier_capteurs(File *file_urgente, Obstacle *obstacle) {
    int obstacle_detecte = (rand() % 200 == 0);
    
    if (obstacle_detecte && !obstacle->active) {
        printf("\n!!! OBSTACLE DETECTE !!!\n");
        
        // Activate obstacle visualization
        obstacle->active = 1;
        obstacle->x = rand() % (SCREEN_WIDTH - 100) + 50;
        obstacle->y = rand() % (SCREEN_HEIGHT - 200) + 150;
        obstacle->timer = 5.0f;
        
        Commande arret = {999, TYPE_ARRET, 1, 0, PRIO_URGENTE};
        ajouter_commande_urgente(file_urgente, arret);

        Commande recul = {998, TYPE_RECULER, 2, 50, PRIO_URGENTE};
        ajouter_commande_urgente(file_urgente, recul);
    }
    
    if (batterie < 20) {
        printf("\n!!! BATTERIE FAIBLE !!!\n");
        Commande recharge = {777, TYPE_RECHARGE, 5, 100, PRIO_URGENTE};
        ajouter_commande_urgente(file_urgente, recharge);
        batterie = 100;
    }
}

int main(void) {
    srand(time(NULL));
    
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Robot Control System - Priority Queue Simulation");
    SetTargetFPS(60);
    
    // Initialize robot
    Robot robot = {
        .x = SCREEN_WIDTH / 2,
        .y = SCREEN_HEIGHT / 2,
        .angle = 0,
        .speed = 0,
        .color = SKYBLUE,
        .is_moving = 0,
        .is_recharging = 0
    };
    
    // Initialize obstacle
    Obstacle obstacle = {0, 0, 0, 0};
    
    // Initialize queues
    File *file_normale = initialiser_file();
    File *file_urgente = initialiser_file();
    
    // Load initial mission
    printf("--- CHARGEMENT DE LA MISSION ---\n");
    ajouter_commande_normale(file_normale, (Commande){1, TYPE_AVANCER, 3, 50, 0});
    ajouter_commande_normale(file_normale, (Commande){2, TYPE_AVANCER, 3, 50, 0});
    ajouter_commande_normale(file_normale, (Commande){3, TYPE_GAUCHE, 2, 30, 0});
    ajouter_commande_normale(file_normale, (Commande){4, TYPE_AVANCER, 5, 80, 0});
    ajouter_commande_normale(file_normale, (Commande){5, TYPE_DROITE, 2, 30, 0});
    ajouter_commande_normale(file_normale, (Commande){6, TYPE_AVANCER, 4, 60, 0});
    
    printf("--- DEMARRAGE DU ROBOT ---\n");
    
    // Main game loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // Update obstacle timer
        if (obstacle.active) {
            obstacle.timer -= deltaTime;
            if (obstacle.timer <= 0) {
                obstacle.active = 0;
            }
        }
        
        // Check sensors periodically
        verifier_capteurs(file_urgente, &obstacle);
        
        // Command execution logic
        if (executing_command) {
            command_timer -= deltaTime;
            
            if (command_timer <= 0) {
                // Command finished
                executing_command = 0;
                robot.is_moving = 0;
                robot.is_recharging = 0;
                robot.color = SKYBLUE;
                
                ecrire_journal(current_command);
                
                if (current_command.type != TYPE_RECHARGE) {
                    batterie -= 15;
                    if (batterie < 0) batterie = 0;
                }
            } else {
                // Continue executing command
                update_robot(&robot, &current_command, deltaTime);
            }
        } else {
            // Get next command
            if (!est_vide(file_urgente)) {
                current_command = prendre_prochaine_commande(file_urgente);
                command_timer = current_command.duree;
                executing_command = 1;
                printf("[URGENT] Execution ID %d\n", current_command.id);
            } else if (!est_vide(file_normale)) {
                current_command = prendre_prochaine_commande(file_normale);
                command_timer = current_command.duree;
                executing_command = 1;
                printf("[NORMAL] Execution ID %d\n", current_command.id);
            }
        }
        
        // Add more commands if queues are getting empty (for continuous demo)
        if (file_normale->nb_elements < 2 && file_urgente->nb_elements == 0) {
            static int id_counter = 100;
            int random_type = (rand() % 4) + 1; // 1-4
            ajouter_commande_normale(file_normale, 
                (Commande){id_counter++, random_type, rand() % 3 + 2, rand() % 50 + 30, 0});
        }
        
        // Drawing
        BeginDrawing();
        ClearBackground((Color){25, 25, 30, 255});
        
        draw_grid();
        draw_obstacle(&obstacle);
        draw_robot(&robot);
        draw_ui(&robot, file_normale, file_urgente, &current_command, executing_command);
        
        // Instructions
        DrawText("Press ESC to exit | Robot navigates autonomously", 10, SCREEN_HEIGHT - 25, 16, DARKGRAY);
        
        EndDrawing();
    }
    
    // Cleanup
    free(file_normale);
    free(file_urgente);
    CloseWindow();
    
    printf("--- MISSION TERMINEE ---\n");
    return 0;
}