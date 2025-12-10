#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#define TYPE_AVANCER 1
#define TYPE_RECULER 2
#define TYPE_GAUCHE  3
#define TYPE_DROITE  4
#define TYPE_ARRET   5
#define TYPE_RECHARGE 6 
#define PRIO_NORMALE 0
#define PRIO_URGENTE 1
typedef struct commande{
    int id;             
    int type;           
    int duree;          
    int vitesse;        
    int priorite;       
} Commande;
typedef struct element_file {
    Commande donnee;
    struct element_file *next;
} Element;
typedef struct File {
    Element *head;
    Element *tail;
    int nb_elements;
} File;
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
int batterie = 100; 

void verifier_capteurs(File *file_urgente) {
    int obstacle_detecte = (rand() % 10 == 0); 
    if (obstacle_detecte) {
        printf("\n!!! OBSTACLE DETECTE !!!\n");
        
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
int main() {
    srand(time(NULL)); 
    File *file_normale = initialiser_file();
    File *file_urgente = initialiser_file();
    printf("--- CHARGEMENT DE LA MISSION ---\n");
    ajouter_commande_normale(file_normale, (Commande){1, TYPE_AVANCER, 3, 50});
    ajouter_commande_normale(file_normale, (Commande){2, TYPE_AVANCER, 3, 50});
    ajouter_commande_normale(file_normale, (Commande){3, TYPE_GAUCHE, 1, 30});
    ajouter_commande_normale(file_normale, (Commande){4, TYPE_AVANCER, 5, 80});
    ajouter_commande_normale(file_normale, (Commande){5, TYPE_DROITE, 1, 30});

    printf("--- DEMARRAGE DU ROBOT ---\n");
    while (!est_vide(file_normale) || !est_vide(file_urgente)) {
        
        verifier_capteurs(file_urgente);

        Commande cmd_actuelle;
        if (!est_vide(file_urgente)) {
            cmd_actuelle = prendre_prochaine_commande(file_urgente);
            printf("[URGENT] Execution ID %d: ", cmd_actuelle.id);
        } else {
            cmd_actuelle = prendre_prochaine_commande(file_normale);
            printf("[NORMAL] Execution ID %d: ", cmd_actuelle.id);
        }
        switch(cmd_actuelle.type) {
            case TYPE_AVANCER: printf("AVANCER (%ds)\n", cmd_actuelle.duree); break;
            case TYPE_RECULER: printf("RECULER (%ds)\n", cmd_actuelle.duree); break;
            case TYPE_GAUCHE:  printf("GAUCHE (%ds)\n", cmd_actuelle.duree); break;
            case TYPE_DROITE:  printf("DROITE (%ds)\n", cmd_actuelle.duree); break;
            case TYPE_ARRET:   printf("ARRET IMMEDIAT !\n"); break;
            case TYPE_RECHARGE: printf("RECHARGE...\n"); break;
        }
        if (cmd_actuelle.type != TYPE_RECHARGE) {
            batterie -= 15;
        }
        ecrire_journal(cmd_actuelle);
    }

    printf("--- MISSION TERMINEE ---\n");
    return 0;
}