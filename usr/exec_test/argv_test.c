int sys_getpid() {
        asm volatile("");
}

int main(int argc, char **argv) {
    printf("Argv Test, pid %d\n", sys_getpid());
    for (int i = 0; i < argc; ++i) {
        puts(argv[i]);
    }
    char *fork_argv[] = {"fork_test", 0};
    sys_exec("fork_test", fork_argv);
}
