#include "install.h"

int promptInstallation(constString srcAddress)
{
    if(!IS_INSTALLED()) // First Installation
    {
        printf(_YEL "The NeoGIT is not found in PATH for launching commands!\n");
        printf(_DFCOLOR _BOLD "Do you want to install NeoGIT in a system folder in PATH? (Y/N): ");
    }
    else // Upgrade
    {
        printf(_YEL "The NeoGIT is already installed in your system!\n");
        printf(_DFCOLOR _BOLD "Do you want to upgrade NeoGIT ? (Y/N): ");
    }    

    char result;
    scanf("%1[YNyn]", &result);
    printf(_RST);

    if(toupper(result) == 'Y')
    {
        printf("\nInstalling NeoGIT in " PROGRAM_PATH " ...\n");
        int res = INSTALL_NEOGIT(srcAddress);
        if(res != 0) perror(_REDB "Failed to install\n\n" _RST);
        else printf(_GRNB "Successfully installed\n\n" _RST);
        return res;
    }
    else 
        printf(_BLU "Installation Cancelled\n\n" _RST);
    return -1;
}

int promptUninstallation()
{
    if(IS_INSTALLED())
    {
        printf(_BOLD _YEL "NeoGIT found in PATH! Are you sure to remove it? (Y/N): " _RST);
        char result;
        scanf("%1[YNyn]", &result);
        if(toupper(result) == 'Y')
        {
            printf("\nRemoving NeoGIT ...\n");
            int res = REMOVE_NEOGIT();
            if(res != 0) perror(_REDB "Error while removing NeoGIT!\n\n" _RST);
            else printf(_GRNB "Removed Successfully!\n\n" _RST);
            return res;
        }
        else
            printf(_BLU "Operation Cancelled\n\n" _RST);
    }
    else printf(_YEL "NeoGIT was not installed!\n\n" _RST);

    return -1;
}

int __install_neogit_linux(constString source)
{
    // Upgrade
    if(IS_INSTALLED()) REMOVE_NEOGIT();

    char __directoryPath[200];
    char* ptr = strrchr(source, '/');
    if (ptr != NULL)
    {
        uint len = (ptr - source);
        sprintf(__directoryPath, "%s", source);
        __directoryPath[len] = '/';
        __directoryPath[len + 1] = '\0';
    }
    else strcpy(__directoryPath, "./");
    
    char _prompt[300];
    sprintf(_prompt, "sudo ln -s \"$(pwd)/%s/" PROGRAM_NAME "\" " PROGRAM_PATH, __directoryPath);
    if(system(_prompt)) return -1;

    return IS_INSTALLED() ? 0 : -1;
}

int __remove_neogit_linux()
{
    if(!IS_INSTALLED()) return 0;
    if(system("sudo rm " PROGRAM_PATH " >/dev/null 2>&1")) return -1;
    return IS_INSTALLED() ? -1 : 0;
}