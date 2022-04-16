#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

FILE *getoutputfile()
{
    static FILE *output_file = NULL;
    if (output_file == NULL)
        output_file = fdopen(atoi(getenv("output_fd")), "w");
    return output_file;
}

void getfilename(FILE *stream, char *path)
{
    char buf[4096];
    memset(path, '\0', sizeof(buf));

    int fd = fileno(stream);
    snprintf(buf, sizeof(buf), "/proc/%d/fd/%d", getpid(), fd);
    readlink(buf, path, sizeof(buf) - 1);
}

void getfilename_fd(int fd, char *path)
{
    char buf[4096];
    memset(path, '\0', sizeof(buf));

    snprintf(buf, sizeof(buf), "/proc/%d/fd/%d", getpid(), fd);
    readlink(buf, path, sizeof(buf) - 1);
}

int chmod(const char *pathname, mode_t mode)
{
    static int (*old_chmod)(const char *pathname, mode_t mode) = NULL;
    if (old_chmod == NULL)
        old_chmod = (int (*)(const char *pathname, mode_t mode))dlsym(RTLD_NEXT, "chmod");

    int re;
    if (old_chmod != NULL)
    {
        re = old_chmod(pathname, mode);
        char path[4096];
        realpath(pathname, path);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] chmod(\"%s\", %.3o) = %d\n", path, mode, re);
    }
    return re;
}

int chown(const char *pathname, uid_t owner, gid_t group)
{
    static int (*old_chown)(const char *pathname, uid_t owner, gid_t group) = NULL;
    if (old_chown == NULL)
        old_chown = (int (*)(const char *pathname, uid_t owner, gid_t group))dlsym(RTLD_NEXT, "chown");

    int re;
    if (old_chown != NULL)
    {
        re = old_chown(pathname, owner, group);
        char path[4096];
        realpath(pathname, path);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] chown(\"%s\", %u, %u) = %d\n", path, owner, group, re);
    }
    return re;
}

int close(int fd)
{
    static int (*old_close)(int fd) = NULL;
    if (old_close == NULL)
        old_close = (int (*)(int fd))dlsym(RTLD_NEXT, "close");

    int re;
    if (old_close != NULL)
    {
        char path[4096];
        getfilename_fd(fd, path);
        re = old_close(fd);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] close(\"%s\") = %d\n", path, re);
    }
    return re;
}

int creat(const char *pathname, mode_t mode)
{
    static int (*old_creat)(const char *pathname, mode_t mode) = NULL;
    if (old_creat == NULL)
        old_creat = (int (*)(const char *pathname, mode_t mode))dlsym(RTLD_NEXT, "creat");

    int re;
    if (old_creat != NULL)
    {
        re = old_creat(pathname, mode);
        char path[4096];
        realpath(pathname, path);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] creat(\"%s\", %.3o) = %d\n", path, mode, re);
    }
    return re;
}

int fclose(FILE *stream)
{
    static int (*old_fclose)(FILE * stream) = NULL;
    if (old_fclose == NULL)
        old_fclose = (int (*)(FILE * stream)) dlsym(RTLD_NEXT, "fclose");

    int re;
    if (old_fclose != NULL)
    {
        char path[4096];
        getfilename(stream, path);

        re = old_fclose(stream);

        FILE *output_file = getoutputfile();

        fprintf(output_file, "[logger] fclose(\"%s\") = %d\n", path, re);
    }
    return re;
}

FILE *fopen(const char *pathname, const char *mode)
{
    static FILE *(*old_fopen)(const char *pathname, const char *mode) = NULL;
    if (old_fopen == NULL)
        old_fopen = (FILE * (*)(const char *pathname, const char *mode)) dlsym(RTLD_NEXT, "fopen");

    FILE *re;
    if (old_fopen != NULL)
    {
        re = old_fopen(pathname, mode);
        char path[4096];
        realpath(pathname, path);

        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] fopen(\"%s\", \"%s\") = %p\n", path, mode, re);
    }
    return re;
}

void getstring(char *str, int size)
{
    for (int i = 0; i < size; i++)
        if (!isprint(str[i]))
            str[i] = '.';
    str[size] = '\0';
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    static int (*old_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
    if (old_fread == NULL)
        old_fread = (int (*)(void *ptr, size_t size, size_t nmemb, FILE *stream))dlsym(RTLD_NEXT, "fread");

    size_t re;
    if (old_fread != NULL)
    {
        re = old_fread(ptr, size, nmemb, stream);
        char str[33];
        int str_size = re;
        if (str_size > 32)
            str_size = 32;
        memcpy(str, ptr, str_size);
        getstring(str, str_size);
        char path[4096];
        getfilename(stream, path);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] fread(\"%s\", %lu, %lu, \"%s\") = %lu\n", str, size, nmemb, path, re);
    }
    return re;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    static int (*old_fwrite)(const void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
    if (old_fwrite == NULL)
        old_fwrite = (int (*)(const void *ptr, size_t size, size_t nmemb, FILE *stream))dlsym(RTLD_NEXT, "fwrite");

    size_t re;
    if (old_fwrite != NULL)
    {
        re = old_fwrite(ptr, size, nmemb, stream);
        char str[33];
        int str_size = size * nmemb;
        if (str_size > 32)
            str_size = 32;
        memcpy(str, ptr, str_size);
        getstring(str, str_size);
        char path[4096];
        getfilename(stream, path);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] fwrite(\"%s\", %lu, %lu, \"%s\") = %lu\n", str, size, nmemb, path, re);
    }
    return re;
}

int open(const char *pathname, int flags, ...)
{
    static int (*old_open)(const char *pathname, int flags, ...) = NULL;
    if (old_open == NULL)
        old_open = (int (*)(const char *pathname, int flags, ...))dlsym(RTLD_NEXT, "open");

    int re;
    if (old_open != NULL)
    {
        mode_t mode = 0;
        if (__OPEN_NEEDS_MODE(flags))
        {
            va_list arg;
            va_start(arg, flags);
            mode = va_arg(arg, mode_t);
            va_end(arg);
        }
        re = old_open(pathname, flags);
        char path[4096];
        realpath(pathname, path);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] open(\"%s\", %.3o, %.3o) = %d\n", path, flags, mode, re);
    }
    return re;
}

ssize_t read(int fd, void *buf, size_t count)
{
    static int (*old_read)(int fd, void *buf, size_t count) = NULL;
    if (old_read == NULL)
        old_read = (int (*)(int fd, void *buf, size_t count))dlsym(RTLD_NEXT, "read");

    ssize_t re;
    if (old_read != NULL)
    {
        re = old_read(fd, buf, count);
        char path[4096];
        getfilename_fd(fd, path);
        char str[33];
        int str_size = re;
        if (str_size > 32)
            str_size = 32;
        memcpy(str, buf, str_size);
        getstring(str, str_size);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] read(\"%s\", \"%s\", %lu) = %lu\n", path, str, count, re);
    }
    return re;
}

int remove(const char *pathname)
{
    static int (*old_remove)(const char *pathname) = NULL;
    if (old_remove == NULL)
        old_remove = (int (*)(const char *pathname))dlsym(RTLD_NEXT, "remove");

    int re;
    if (old_remove != NULL)
    {
        re = old_remove(pathname);
        char path[4096];
        realpath(pathname, path);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] remove(\"%s\") = %d\n", path, re);
    }
    return re;
}

int rename(const char *oldpath, const char *newpath)
{
    static int (*old_rename)(const char *oldpath, const char *newpath) = NULL;
    if (old_rename == NULL)
        old_rename = (int (*)(const char *oldpath, const char *newpath))dlsym(RTLD_NEXT, "rename");

    int re;
    if (old_rename != NULL)
    {
        re = old_rename(oldpath, newpath);
        char path[4096], path2[4096];
        realpath(oldpath, path);
        realpath(newpath, path2);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] rename(\"%s\", \"%s\") = %d\n", path, path2, re);
    }
    return re;
}

FILE *tmpfile(void)
{

    static FILE *(*old_tmpfile)(void) = NULL;
    if (old_tmpfile == NULL)
        old_tmpfile = (FILE * (*)(void)) dlsym(RTLD_NEXT, "tmpfile");

    FILE *re;
    if (old_tmpfile != NULL)
    {
        char path[4096];
        re = old_tmpfile();
        getfilename(re, path);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] tmpfile() = %p\n", re);
    }
    return re;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    static int (*old_write)(int fd, const void *buf, size_t count) = NULL;
    if (old_write == NULL)
        old_write = (int (*)(int fd, const void *buf, size_t count))dlsym(RTLD_NEXT, "write");

    ssize_t re;
    if (old_write != NULL)
    {
        re = old_write(fd, buf, count);
        char path[4096];
        getfilename_fd(fd, path);
        char str[33];
        int str_size = count;
        if (str_size > 32)
            str_size = 32;
        memcpy(str, buf, str_size);
        getstring(str, str_size);
        FILE *output_file = getoutputfile();
        fprintf(output_file, "[logger] write(\"%s\", \"%s\", %lu) = %ld\n", path, str, count, re);
    }
    return re;
}