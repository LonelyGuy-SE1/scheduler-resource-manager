#include <stdio.h>
#include "campus.h"

static void print_menu(void)
{
    printf("\n========== CampusCompute ==========\n");
    printf("1. Job scheduling (FCFS / RR / Priority)\n");
    printf("2. Concurrent activity logger\n");
    printf("3. Shared-memory IPC\n");
    printf("4. Banker's algorithm (safe / unsafe)\n");
    printf("5. Worker success & failure\n");
    printf("6. Resource limit demo\n");
    printf("0. Exit\n");
    printf("===================================\n");
    printf("Enter choice: ");
}

int main(void)
{
    int choice;

    printf("CampusCompute — shared lab OS concepts prototype\n");

    for (;;) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
            }
            printf("Invalid input. Enter a menu number (0-6).\n");
            continue;
        }

        switch (choice) {
        case 1:
            run_scheduling();
            break;
        case 2:
            run_logger();
            break;
        case 3:
            run_ipc();
            break;
        case 4:
            run_banker();
            break;
        case 5:
            run_workers();
            break;
        case 6:
            run_limits();
            break;
        case 0:
            printf("Goodbye.\n");
            return 0;
        default:
            printf("Invalid option. Choose 0-6.\n");
            break;
        }
    }
}
