# C-Shell - Custom Unix Shell Implementation

A comprehensive Unix shell implementation written in C that provides enhanced functionality including background process management, piping, command history, file searching, process monitoring, and custom shell scripting capabilities.

## 🎯 Project Overview

C-Shell is a feature-rich command-line interpreter that implements many standard Unix shell features along with several custom enhancements. It provides a robust environment for process management, file operations, and system monitoring with additional utilities for modern shell usage.

## ✨ Key Features

### 🔧 **Core Shell Functionality**
- **Interactive Command Line Interface** with colored output
- **Background & Foreground Process Management**
- **Command History & Logging** with persistent storage
- **Pipe Support** for command chaining
- **Signal Handling** for process control
- **Custom Prompt** with directory and user information

### 📁 **File & Directory Operations**
- **Enhanced Directory Navigation** (`hop` command)
- **Detailed File Listing** (`reveal` command with ls-like functionality)
- **Advanced File Search** (`seek` command with recursive search)
- **Path Management** with home directory awareness

### 🔍 **Process & System Monitoring**
- **Process Information** (`proclore` command)
- **Background Process Tracking** (`activities` command)
- **Real-time Process Monitoring** (`neonate` command)
- **Process Control** (`ping`, `fg`, `bg` commands)

### 📚 **Enhanced Features**
- **Command Aliases** and custom functions via `.myshrc`
- **Manual Pages** (`iMan` command)
- **Persistent Command History** across sessions
- **Auto-completion** support
- **Error Handling** with graceful recovery

## 🏗️ Architecture

### File Structure
```
c-shell/
├── myshell.c/.h          # Main shell implementation
├── headers.h             # Common headers and definitions
├── Makefile             # Build configuration
├── fcom.txt             # Built-in commands list
├── myshrc               # Shell configuration file
├── log.txt              # Command history storage
│
├── Core Modules:
├── input.c/.h           # Input handling and parsing
├── handlers.c/.h        # Command dispatch and execution
├── pipes.c/.h           # Pipe implementation
├── bg_handler.c/.h      # Background process management
│
├── Built-in Commands:
├── dir.c/.h             # Directory operations (hop)
├── reveal.c/.h          # File listing (reveal)
├── log.c/.h             # Command history (log)
├── seek.c/.h            # File search (seek)
├── proclore.c/.h        # Process information
├── activities.c/.h      # Background process listing
├── ping.c/.h            # Signal sending
├── fg.c/.h              # Foreground process control
├── neonate.c/.h         # Real-time process monitoring
├── iman.c/.h            # Manual pages
└── myshrc_parser.c/.h   # Configuration file parsing
```

## 🛠️ Built-in Commands

### 1. **hop** - Enhanced Directory Navigation
```bash
hop                    # Go to home directory
hop ~                  # Go to home directory  
hop -                  # Go to previous directory
hop /path/to/dir       # Go to specified directory
hop dir1 dir2 dir3     # Navigate through multiple directories
```

### 2. **reveal** - Advanced File Listing
```bash
reveal                 # List files in current directory
reveal -l              # Detailed listing (like ls -l)
reveal -a              # Show hidden files
reveal -la             # Detailed listing with hidden files
reveal /path           # List files in specified path
```

### 3. **log** - Command History Management
```bash
log                    # Show command history
log execute 3          # Execute 3rd recent command
log purge              # Clear command history
```

### 4. **proclore** - Process Information
```bash
proclore               # Show info for current shell
proclore 1234          # Show info for process ID 1234
```

### 5. **seek** - Advanced File Search
```bash
seek filename          # Search for file/directory
seek -d dirname        # Search only directories
seek -f filename       # Search only files
seek -e filename       # Execute if single match found
```

### 6. **activities** - Background Process Monitor
```bash
activities             # List all background processes
```

### 7. **ping** - Process Signal Control
```bash
ping 1234 9            # Send SIGKILL to process 1234
ping 1234 19           # Send SIGSTOP to process 1234
```

### 8. **fg** - Foreground Process Control
```bash
fg 1234                # Bring background process to foreground
```

### 9. **bg** - Background Process Control
```bash
bg 1234                # Continue stopped process in background
```

### 10. **neonate** - Real-time Process Monitoring
```bash
neonate -n 2           # Show most recent process every 2 seconds
# Press 'x' to exit
```

### 11. **iMan** - Manual Pages
```bash
iMan ls                # Show manual for 'ls' command
iMan grep              # Show manual for 'grep' command
```

## 🔧 Advanced Features

### Background Process Management
```bash
sleep 10 &             # Run command in background
firefox &              # Launch application in background
activities             # View all background processes
```

### Piping Support
```bash
ls | grep txt          # Basic piping
cat file.txt | grep pattern | wc -l   # Multiple pipes
ls | grep txt &        # Background piping
```

### Shell Configuration (.myshrc)
```bash
# Aliases
reveall rl = reveal -l
home = hop ~

# Custom Functions
func mkhop () {
    mkdir "$1"
    hop "$1"
}

func hop_seek () {
    hop "$1"
    seek "$2"
}
```

## 🚀 Installation & Usage

### Prerequisites
- **Linux/Unix Environment** (tested on Ubuntu/Debian)
- **GCC Compiler** (C99 or later)
- **Standard POSIX Libraries**

### Build Instructions
```bash
# Clone the repository
git clone <repository-url>
cd c-shell

# Compile the shell
make

# Run the shell
./a.out
```

### Alternative Build
```bash
# Manual compilation
gcc -o myshell *.c

# Run with debug information
gcc -g -o myshell *.c
```

### Usage
```bash
# Start the shell
./a.out

# The shell will display a custom prompt
username@hostname:current_directory> 

# Exit the shell
exit
# or press Ctrl+D
# or type 'q' and press Enter
```

## 🎨 Features in Detail

### 1. **Smart Command History**
- Maintains last 15 commands
- Persistent across shell sessions
- Stored in `log.txt` file
- Execute previous commands by index

### 2. **Enhanced Directory Management**
- Tracks current, previous, and home directories
- Supports relative and absolute paths
- Tilde (`~`) expansion for home directory
- Previous directory access with `-`

### 3. **Background Process Handling**
- Automatic background process management
- Process status tracking (Running/Stopped)
- Signal handling for process control
- Clean process termination

### 4. **Advanced File Operations**
- Recursive file search with pattern matching
- Detailed file information display
- Hidden file support
- Permission and ownership information

### 5. **Real-time Monitoring**
- Live process monitoring with `neonate`
- Background process activity tracking
- System resource information
- Process state monitoring

## 🔐 Security Features

- **Input Validation** - Prevents buffer overflows and injection attacks
- **Safe Memory Management** - Proper allocation and deallocation
- **Error Handling** - Graceful error recovery without crashes
- **Process Isolation** - Proper forking and process management
- **Signal Safety** - Secure signal handling

## 📋 Command Reference

### Signal Numbers for `ping` Command
```bash
ping <pid> 1    # SIGHUP - Hangup
ping <pid> 2    # SIGINT - Interrupt (Ctrl+C)
ping <pid> 9    # SIGKILL - Kill (cannot be caught)
ping <pid> 15   # SIGTERM - Terminate
ping <pid> 18   # SIGCONT - Continue
ping <pid> 19   # SIGSTOP - Stop (cannot be caught)
ping <pid> 20   # SIGTSTP - Terminal stop (Ctrl+Z)
```

### File Type Indicators in `reveal`
```
-  Regular file
d  Directory
l  Symbolic link
c  Character device
b  Block device
p  Named pipe (FIFO)
s  Socket
```

## 🐛 Error Handling

The shell implements comprehensive error handling:

- **Invalid Commands** - Clear error messages for unknown commands
- **Permission Errors** - Graceful handling of access denied scenarios  
- **File Not Found** - Informative messages for missing files/directories
- **Process Errors** - Proper handling of process creation failures
- **Memory Errors** - Safe memory allocation with error checking
- **Signal Errors** - Robust signal handling and cleanup

## 🔍 Troubleshooting

### Common Issues

#### Shell Won't Start
```bash
# Check compilation errors
make clean && make

# Verify executable permissions
chmod +x a.out
```

#### Commands Not Working
```bash
# Check if command exists in fcom.txt
cat fcom.txt

# Verify PATH is set correctly
echo $PATH
```

#### Background Processes Not Tracked
```bash
# Check if process was started with &
sleep 10 &

# Verify activities command
activities
```

#### History Not Saving
```bash
# Check log file permissions
ls -la log.txt

# Verify write permissions in shell directory
```

### Possible Improvements
- **Performance Optimization** - Faster command parsing
- **Memory Efficiency** - Reduced memory footprint
- **Error Recovery** - Better error handling mechanisms
- **Logging** - Enhanced logging and debugging features
- **Portability** - Support for different Unix variants

## 📚 Implementation Details

### Key Data Structures
```c
// Background process tracking
typedef struct bg_process {
    int pid;
    char *name;
    char *command_name;
    int status;
    struct bg_process *next, *prev;
} bg_process;

// Command history
typedef struct log_deque {
    int cap;
    int size;
    char **arr;
} log_deque;

// Directory tree management
typedef struct dir_node {
    char *name;
    bool is_home;
    int num_children;
    struct dir_node **children;
    struct dir_node *parent;
} dir_node;
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-feature`)
3. Commit your changes (`git commit -am 'Add new feature'`)
4. Push to the branch (`git push origin feature/new-feature`)
5. Create a Pull Request
