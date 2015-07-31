# Start up a parent-child example of using exec and a pipe.
# The messages print the pid, which is nice :)
set -x
gcc pipe_exec_parent.c
gcc pipe_exec_child.c -o pipe_exec_child
./a.out
sleep 1  # without this, it seems sometimes the child doesn't run.
         # my guess is, this is what happens:
         #   bash does indeed block as long as ./a.out is running
         #   but, as soon as the parent process exits, bash resumes. It doesn't
         #     wait for grandchild processes to finish
         #   as a result, it's possible to hit the rm statement before the
         #     child process has had a chance to call execl()
         #   putting this sleep 1 here fixes the issue
rm pipe_exec_child
