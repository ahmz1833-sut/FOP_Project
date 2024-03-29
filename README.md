# NeoGIT

NeoGIT is a lightweight and user-friendly VCS, built with simplicity and efficiency in mind. It provides a set of essential Git commands in a user-friendly interface, making it easier for developers to manage their repositories.
* Note: _This release of neogit only works in POSIX based systems._

Features
* Intuitive Command Line Interface: NeoGIT offers a straightforward command-line interface that simplifies common Git operations.

* Branch Management: Easily create, switch, and merge branches to streamline your development workflow.

* Commit: Commit changes in repository with just a few simple commands.

* Tagging: Manage tags to mark specific points in your project's history for easy reference.

* Diff Viewer: Visualize the differences between files or commits to understand changes more effectively.

* Grep Functionality: Search for specific patterns or words within your project files with the powerful grep command.

* Merge Operation: Merge branches. (without conflict resolution)

## Usage/Examples

For launching NeoGIT in your system, you must compile the project into one executable file neogit. Then make symlink or copy it to `/usr/local/bin` . 

* ### Initializing Repository
    ```
    neogit init
    ```
    Use command above to init the neogit repository with default branch `master` in your current working directory.

* ### Branches
	To switch to a different branch, you can use the `neogit checkout` command. This command allows you to switch to a different branch.
	To list all available branches in a Neogit repository, you can use the `neogit branch` command. This command will display a list of all branches in the repository, along with their current status (e.g. whether they are ahead or behind the current branch).

* ### Commit Logs 
  * Show commit logs  (`neogit log`)
  
  Log command has useful find tools for filtering commits, you can see this options by `neogit log --help`

* ### Help and Syntax of commands
	You can use `--help` after each command to see  its help message. For example: `neogit status --help`.

## Author

- AmirHossein MohammadZadeh 402106434 (CE. student at SUT) [@ahmz1833-sut](https://github.com/ahmz1833-sut)

I'm glad to hear that you found this tutorial helpful! If you have any other questions about Neogit or Git in general, don't hesitate to ask. I'll do my best to help.

In the meantime, I would encourage you to continue exploring Neogit and learning more about how it can help you manage your Git repositories. It's a powerful tool that can make working with Git much easier and more efficient.