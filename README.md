## Assumptions
1. log command will take in and store non-empty non-valid commands too.
2. the input is parsed using strtok which rarely tends to tokenize incorrectly. In such cases an Error output is shown.
It is recommended to re-run the terminal.

Max Limits :
- filenames : 256 bytes 
- paths : 512 bytes
- command_lengths : 1024 bytes
- commands w/o paths : 256 bytes
- processes : 128 processes
- commands : 256 commands

3. If an internal error occurs on the execution of any input, then also the input will be saved on the log.
4. seek is assumed to be on the folders except (. and ..)
5. error is printed for every file which raises some exception while doing seek.

6. The virtual memory reported in the proclore call is in bytes.
7. All errors are handled such that they won't lead to the crashing of the shell..
8. However, when an error occurs in the initialization of the shell, then it is reflected and the shell exits with non-zero exit code.