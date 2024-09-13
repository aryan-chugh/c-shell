## Assumptions
1. log command will take in and store non-empty non-valid commands too.
2. If an internal error occurs on the execution of any input, then also the input will be saved on the log.
3. seek is assumed to be on the folders except (. and ..)
4. error is printed for every file which raises some exception while doing seek.

5. The virtual memory reported in the proclore call is in bytes.
6. All errors are handled such that they won't lead to the crashing of the shell..
7. However, when an error occurs in the initialization of the shell, then it is reflected and the shell exits with non-zero exit code.

8. For this version of the submission we can terminate the shell by pressing q and `Enter`
9. seek with no search name will match all the files .. as all files can be considered to start with empty "".
10. seek matches files / directories with prefix as the search string.

11. syntax requirements for myshrc functions :
    - the 'func' keyword must be specified.
    - the name of the function must be separated from the () or func by at least one space.
    - there must be atleast one space after () and before '{' or '\n'
12. implementation details of aliasing :
    - `reveall reveal_l = reveal -l` if given then it would take the alias of both reveall and reveal_l as reveal -l
13. Assumption:
    - neonate -n uses the /stat/ directory of the linux system to check for the most recently created process... 
    - Now, if the time interval is relatively short then, there may arise some file opening problem for the newest created file, it happens for 1-2 attempts but may vary depending on how short the time interval is. Then it correctly reads and outputs.

    Update : 
    - Used this in my logic, if there is a file created and it cannot be read then it has to be corresponding to the most recently created process...
14. `activities` will give all states as Running except T which will be reported as Stopped.

------------------
updates : 
--- necessary changes to proclore
--- piping implemented.
--- background process handling changed.


----------------------


