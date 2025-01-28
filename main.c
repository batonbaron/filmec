#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <locale.h>
#include <windows.h>


typedef struct Film {
    char name[100];
    int releaseYear;
    char origin[100];
    char category[100];
    float score;
    struct Film* nextFilm;
    struct Film* prevFilm;
} Film;

typedef struct Account {
    char login[100];
    char pass[100];
    char payment[20];
    int favCount;
    int adminStatus;
} Account;

Account activeUser;

Film* makeFilm(const char* name, int releaseYear, const char* origin, const char* category, float score) {
    Film* newFilm = (Film*)malloc(sizeof(Film));
    strcpy(newFilm->name, name);
    newFilm->releaseYear = releaseYear;
    strcpy(newFilm->origin, origin);
    strcpy(newFilm->category, category);
    newFilm->score = score;
    newFilm->nextFilm = NULL;
    newFilm->prevFilm = NULL;
    return newFilm;
}

void insertFilm(Film** start, Film* newFilm) {
    if (*start == NULL) {
        *start = newFilm;
        newFilm->nextFilm = newFilm;
        newFilm->prevFilm = newFilm;
    }
    else {
        Film* last = (*start)->prevFilm;
        last->nextFilm = newFilm;
        newFilm->prevFilm = last;
        newFilm->nextFilm = *start;
        (*start)->prevFilm = newFilm;
    }
}

void deleteFilm(Film** start, Film* film) {
    if (*start == NULL || film == NULL) return;

    if (*start == film && (*start)->nextFilm == *start) {
        *start = NULL;
    }
    else {
        Film* prev = film->prevFilm;
        Film* next = film->nextFilm;
        prev->nextFilm = next;
        next->prevFilm = prev;
        if (*start == film) {
            *start = next;
        }
    }
    free(film);
}

void storeFilms(Film* start) {
    FILE* file = fopen("films.txt", "w");
    if (file == NULL) {
        perror("Failed to open films file");
        return;
    }

    Film* current = start;
    do {
        fprintf(file, "%s\n%d\n%s\n%s\n%f\n", current->name, current->releaseYear, current->origin, current->category, current->score);
        current = current->nextFilm;
    } while (current != start);

    fclose(file);
}

char* createString(const char* input, int length) {
    char* output = (char*)malloc((length * 2 + 1) * sizeof(char));
    int index = 0;
    int count = 0;
    int outputIndex = 0;

    while (input[index] != '\0' && count < length) {
        unsigned char c = (unsigned char)input[index];
        int size = 1;

        if (c >= 0xC0) {
            size = 2;
        }

        if (count + 1 <= length) {
            for (int i = 0; i < size; i++) {
                if (input[index] == '\0') break;
                output[outputIndex++] = input[index++];
            }
            count++;
        }
        else {
            break;
        }
    }

    while (count < length) {
        output[outputIndex++] = ' ';
        count++;
    }

    output[outputIndex] = '\0';
    return output;
}

int checkFav(Film* film) {
    char filepath[120];
    snprintf(filepath, sizeof(filepath), "favorites_%s.txt", activeUser.login);
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        return 0;
    }

    char name[100];
    int year;
    char origin[100];
    char category[100];
    float score;
    char buffer[100];

    while (fgets(name, sizeof(name), file)) {
        name[strcspn(name, "\n")] = '\0';
        fgets(buffer, sizeof(buffer), file);
        sscanf(buffer, "%d", &year);
        fgets(origin, sizeof(origin), file);
        origin[strcspn(origin, "\n")] = '\0';
        fgets(category, sizeof(category), file);
        category[strcspn(category, "\n")] = '\0';
        fgets(buffer, sizeof(buffer), file);
        sscanf(buffer, "%f", &score);

        if (strcmp(name, film->name) == 0 && year == film->releaseYear && strcmp(origin, film->origin) == 0 && strcmp(category, film->category) == 0 && score == film->score) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

void updateAccountFile() {
    FILE* file = fopen("users.txt", "r");
    if (file == NULL) {
        perror("Failed to open users file");
        return;
    }

    FILE* tempFile = fopen("temp_users.txt", "w");
    if (tempFile == NULL) {
        perror("Failed to open temp file");
        fclose(file);
        return;
    }

    char login[100];
    char pass[100];
    char payment[20];
    int favCount;
    int adminStatus;

    while (fscanf(file, "%s %s %s %d %d", login, pass, payment, &favCount, &adminStatus) != EOF) {
        if (strcmp(login, activeUser.login) == 0) {
            fprintf(tempFile, "%s %s %s %d %d\n", activeUser.login, activeUser.pass, activeUser.payment, activeUser.favCount, activeUser.adminStatus);
        }
        else {
            fprintf(tempFile, "%s %s %s %d %d\n", login, pass, payment, favCount, adminStatus);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove("users.txt");
    rename("temp_users.txt", "users.txt");
}

void saveFavFilm(Film* film) {
    if (checkFav(film)) {
        printf("Film already in favorites!\n");
        return;
    }

    char filepath[120];
    snprintf(filepath, sizeof(filepath), "favorites_%s.txt", activeUser.login);
    FILE* file = fopen(filepath, "a");
    if (file == NULL) {
        perror("Failed to open favorites file");
        return;
    }
    fprintf(file, "%s\n%d\n%s\n%s\n%f\n", film->name, film->releaseYear, film->origin, film->category, film->score);
    fclose(file);
    activeUser.favCount++;
    updateAccountFile();
}

void removeFavFilm(Film* film) {
    char filepath[120];
    snprintf(filepath, sizeof(filepath), "favorites_%s.txt", activeUser.login);
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        perror("Failed to open favorites file");
        return;
    }

    FILE* tempFile = fopen("temp.txt", "w");
    if (tempFile == NULL) {
        perror("Failed to open temp file");
        fclose(file);
        return;
    }

    char name[100];
    int year;
    char origin[100];
    char category[100];
    float score;
    char buffer[100];

    while (fgets(name, sizeof(name), file)) {
        name[strcspn(name, "\n")] = '\0';
        fgets(buffer, sizeof(buffer), file);
        sscanf(buffer, "%d", &year);
        fgets(origin, sizeof(origin), file);
        origin[strcspn(origin, "\n")] = '\0';
        fgets(category, sizeof(category), file);
        category[strcspn(category, "\n")] = '\0';
        fgets(buffer, sizeof(buffer), file);
        sscanf(buffer, "%f", &score);

        if (strcmp(name, film->name) != 0 || year != film->releaseYear || strcmp(origin, film->origin) != 0 || strcmp(category, film->category) != 0 || score != film->score) {
            fprintf(tempFile, "%s\n%d\n%s\n%s\n%f\n", name, year, origin, category, score);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(filepath);
    rename("temp.txt", filepath);
    activeUser.favCount--;
    updateAccountFile();
}

int validatePayment(const char* payment) {
    int len = strlen(payment);
    if (len != 16) return 0;
    for (int i = 0; i < len; i++) {
        if (!isdigit(payment[i])) return 0;
    }
    return 1;
}

void showFilms(Film* start, int isFavList, const char* header) {
    if (start == NULL) {
        if (isFavList) {
            printf("Favorites list is empty.\n");
            printf("Press any key to return to the menu...");
            _getch();
        }
        return;
    }

    Film* current = start;
    while (1) {
        system("cls");

        if (header == "Фильмы") {
            printf("                                                   === Фильмы ===\n");
        }
        else if (header == "Избранное") {
            printf("                                                 === Избранное ===\n");
        }

        char* prevName = createString(current->prevFilm->name, 30);
        char* currentName = createString(current->name, 40);
        char* nextName = createString(current->nextFilm->name, 30);
        char* currentOrigin = createString(current->origin, 32);
        char* currentCategory = createString(current->category, 34);

        printf("+--------------------------------+  +------------------------------------------+  +--------------------------------+\n");
        printf("| %s |  ", prevName);
        printf("| %s |  ", currentName);
        printf("| %s |\n", nextName);
        printf("| Год: %-25d |  ", current->prevFilm->releaseYear);
        printf("| Год: %-35d |  ", current->releaseYear);
        printf("| Год: %-25d |\n", current->nextFilm->releaseYear);
        printf("+--------------------------------+  | Страна: %s |  +--------------------------------+\n", currentOrigin);
        printf("                                    ");
        printf("| Жанр: %s |\n", currentCategory);
        printf("                                    ");
        printf("| Рейтинг: %-31.1f |\n", current->score);
        if (checkFav(current)) {
            printf("                                    | В избранном: Да                          |\n");
        }
        else {
            printf("                                    | В избранном: Нет                         |\n");
        }
        printf("                                    +------------------------------------------+\n");

        free(prevName);
        free(currentName);
        free(nextName);
        free(currentOrigin);
        free(currentCategory);

        printf("\n q - выход в меню, ");
        if (isFavList) {
            printf("r - удалить из избранного");
        }
        else {
            printf("f - добавить в список избранных, r - удалить из избранного");
            if (activeUser.adminStatus) {
                printf(", g - удалить фильм");
            }
        }
        printf("\n");

        char ch = _getch();
        if (ch == 'a') {
            current = current->prevFilm;
        }
        else if (ch == 'd') {
            current = current->nextFilm;
        }
        else if (ch == 'f' && !isFavList) {
            saveFavFilm(current);
        }
        else if (ch == 'r') {
            removeFavFilm(current);
        }
        else if (ch == 'g' && activeUser.adminStatus) {
            deleteFilm(&start, current);
            storeFilms(start);
            break;
        }
        else if (ch == 'q') {
            break;
        }
    }
}

void freeFilms(Film* start) {
    if (start == NULL) return;
    Film* current = start;
    do {
        Film* temp = current;
        current = current->nextFilm;
        free(temp);
    } while (current != start);
}

int createAccount(const char* login, const char* pass, const char* payment) {
    FILE* file = fopen("users.txt", "a");
    if (file == NULL) {
        perror("Failed to open users file");
        return 0;
    }
    fprintf(file, "%s %s %s 0 0\n", login, pass, payment);
    fclose(file);
    return 1;
}

int verifyAccount(const char* login, const char* pass) {
    FILE* file = fopen("users.txt", "r");
    if (file == NULL) {
        perror("Failed to open users file");
        return 0;
    }
    char storedLogin[100];
    char storedPass[100];
    char storedPayment[20];
    int favCount;
    int adminStatus;
    while (fscanf(file, "%s %s %s %d %d", storedLogin, storedPass, storedPayment, &favCount, &adminStatus) != EOF) {
        if (strcmp(login, storedLogin) == 0 && strcmp(pass, storedPass) == 0) {
            fclose(file);
            strcpy(activeUser.login, storedLogin);
            strcpy(activeUser.pass, storedPass);
            strcpy(activeUser.payment, storedPayment);
            activeUser.favCount = favCount;
            activeUser.adminStatus = adminStatus;
            return 1;
        }
    }
    fclose(file);
    return 0;
}

void loginScreen() {
    char login[100];
    char pass[100];
    char payment[20];
    int option;

    while (1) {
        system("cls");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        if (option == 1) {
            printf("Enter login: ");
            scanf("%s", login);
            printf("Enter password: ");
            scanf("%s", pass);
            while (1) {
                printf("Enter payment number (16 digits): ");
                scanf("%s", payment);
                if (validatePayment(payment)) {
                    break;
                }
                else {
                    printf("Invalid payment number. Please try again.\n");
                }
            }
            if (createAccount(login, pass, payment)) {
                printf("Registration successful!\n");
            }
            else {
                printf("Registration failed!\n");
            }
        }
        else if (option == 2) {
            printf("Enter login: ");
            scanf("%s", login);
            printf("Enter password: ");
            scanf("%s", pass);
            if (verifyAccount(login, pass)) {
                printf("Login successful!\n");
                break;
            }
            else {
                printf("Invalid login or password!\n");
            }
        }
        else if (option == 3) {
            exit(0);
        }
        else {
            printf("Invalid choice!\n");
        }
        printf("Press any key to continue...");
        _getch();
    }
}

void loadFavorites(Film** start) {
    char filepath[120];
    snprintf(filepath, sizeof(filepath), "favorites_%s.txt", activeUser.login);
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        return;
    }

    char name[100];
    int year;
    char origin[100];
    char category[100];
    float score;
    char buffer[100];

    while (fgets(name, sizeof(name), file)) {
        name[strcspn(name, "\n")] = '\0';
        fgets(buffer, sizeof(buffer), file);
        sscanf(buffer, "%d", &year);
        fgets(origin, sizeof(origin), file);
        origin[strcspn(origin, "\n")] = '\0';
        fgets(category, sizeof(category), file);
        category[strcspn(category, "\n")] = '\0';
        fgets(buffer, sizeof(buffer), file);
        sscanf(buffer, "%f", &score);

        Film* newFilm = makeFilm(name, year, origin, category, score);
        insertFilm(start, newFilm);
    }

    fclose(file);
}

void addFilm(Film** start) {
    char name[100];
    int year;
    char origin[100];
    char category[100];
    float score;

    printf("Enter film name: ");
    scanf(" %[^\n]", name);
    printf("Enter film year: ");
    scanf("%d", &year);
    printf("Enter film origin: ");
    scanf(" %[^\n]", origin);
    printf("Enter film category: ");
    scanf(" %[^\n]", category);
    printf("Enter film score: ");
    scanf("%f", &score);

    Film* newFilm = makeFilm(name, year, origin, category, score);
    insertFilm(start, newFilm);
    storeFilms(*start);
}

void menu(Film** start) {
    int option;

    while (1) {
        system("cls");
        printf("1. View all films\n");
        printf("2. View favorite films\n");
        if (activeUser.adminStatus) {
            printf("3. Add new film\n");
        }
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        if (option == 1) {
            showFilms(*start, 0, "Фильмы");
        }
        else if (option == 2) {
            Film* favorites = NULL;
            loadFavorites(&favorites);
            showFilms(favorites, 1, "Избранное");
            freeFilms(favorites);
        }
        else if (option == 3 && activeUser.adminStatus) {
            addFilm(start);
        }
        else if (option == 4 || (option == 3 && !activeUser.adminStatus)) {
            break;
        }
        else {
            printf("Invalid choice!\n");
        }
        printf("Press any key to continue...");
        _getch();
    }
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");

    loginScreen();

    FILE* file = fopen("films.txt", "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    Film* start = NULL;
    char name[100];
    int year;
    char origin[100];
    char category[100];
    float score;
    char buffer[100];

    while (fgets(name, sizeof(name), file)) {
        name[strcspn(name, "\n")] = '\0';
        fgets(buffer, sizeof(buffer), file);
        sscanf(buffer, "%d", &year);
        fgets(origin, sizeof(origin), file);
        origin[strcspn(origin, "\n")] = '\0';
        fgets(category, sizeof(category), file);
        category[strcspn(category, "\n")] = '\0';
        fgets(buffer, sizeof(buffer), file);
        sscanf(buffer, "%f", &score);

        Film* newFilm = makeFilm(name, year, origin, category, score);
        insertFilm(&start, newFilm);
    }

    fclose(file);

    menu(&start);
    freeFilms(start);

    return 0;
}