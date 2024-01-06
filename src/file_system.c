#include "file_system.h"


int runAsAdmin(String command)
{
    #ifdef _WIN32 // For Windows
        HINSTANCE hInstance = ShellExecute(NULL, "runas", command, NULL, NULL, SW_SHOWNORMAL);

        if ((int)hInstance <= 32) {
            // ShellExecute failed, handle the error here
            // Error codes: https://docs.microsoft.com/en-us/windows/win32/debug/system-error-codes
            // Example: if ((int)hInstance == ERROR_FILE_NOT_FOUND) { /* handle error */ }
        } else {
            // Command executed successfully
        }
    #else // For Linux
        // Build the full command with sudo
        char fullCommand[256];
        sprintf(fullCommand, "sudo %s", command);

        // Execute the command
        int returnValue = system(fullCommand);

        if (returnValue == -1) {
            perror("Error executing command");
            return EXIT_FAILURE;
        } else if (WIFEXITED(returnValue)) {
            int exitStatus = WEXITSTATUS(returnValue);

            if (exitStatus == 0) {
                printf("Command executed successfully.\n");
                return EXIT_SUCCESS;
            } else {
                fprintf(stderr, "Command failed with exit status %d\n", exitStatus);
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "Command did not terminate successfully.\n");
            return EXIT_FAILURE;
        }
    #endif
}

#ifdef _WIN32
// For Windows

#else
// For Linux

#endif