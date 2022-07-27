![alt text](https://github.com/adam-p/markdown-here/raw/master/src/common/images/icon48.png "Logo Title Text 1") 
# Syscall-project 
---
![alt_text](https://upload.wikimedia.org/wikipedia/commons/d/dd/Linux_logo.jpg)

---
### SOFTWARE SPECIFICATION
- Go inside project and run make in bash session
- Run ./server in the same session
- In other bash session run ./client0 path-to-folder 
- In client session press ctrl-c (SIGINT signal)
- Server will produce "_out" file in "path-to-folder"
- Run ctrl-c more times to refresh "_out file"

In this project the folder "tester_folder", is used to test the application. 
If fifo's or IPCS already exists run ./clean.sh, this script remove all IPCS
