#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <dirent.h>
#include "ansi_color.h"

#define CHECK_EXIST(s) (system("ls " s " >/dev/null 2>&1") == 0)

int main(int argc, char const *argv[])
{
    prepareANSI();

    printf("The Number of arguments is : \"%d\" \n", argc);
    for(int i = 0; i < argc; i++) printf("The Argument[%d] is : \"%s\" \n", i, argv[i]);

    if(argc == 2 && !strcmp(argv[1], "--install"))
    {
        if(!CHECK_EXIST("/bin/neogit"))
        {
            char result;
            printf("The NeoGIT is not found in /bin/ for launching commands!\n");
            printf("Do you want to make a symbolic link to this file in /bin/ ? (Y/N): ");
            scanf("%1[YNyn]", &result);
            if(toupper(result) == 'Y')
            {
                printf("Installing NeoGIT in system PATH ...\n");
                int res = system("sudo ln -s \"$(pwd)/neogit\" /bin/neogit");
                if(res != 0) perror(_SGR_REDF "Failed to install\n" _SGR_DEF_FG);
                else printf(_SGR_GREENF "Successfully installed\n" _SGR_DEF_FG);
            }
            else exit(0);
        }
        else
            printf("The Program already is in the PATH.\n");
    }

    else if(argc == 2 && !strcmp(argv[1], "--uninstall"))
    {
        if(CHECK_EXIST("/bin/neogit"))
        {
            printf("NeoGIT found in PATH! Removing it ...\n");
            int res = system("sudo rm /bin/neogit >/dev/null 2>&1");
            if(res != 0) perror("Error while removing from PATH!\n");
            else printf("Removed Successfully!\n");
        }
        else printf("NeoGIT was not found in PATH!\n");
    }

    int i = 0;
    scanf("%d", &i);
    printf("salam %d\n", i);
    return 0;
}
