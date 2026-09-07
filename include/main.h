#ifndef MAIN_H
#define MAIN_H

typedef enum
{
    false,
    true
} bool;

extern char *shader_bin_read(const char *file)
{
    char path[128];
    snprintf(path, sizeof(path), "shaders/build/%s", file);

    FILE *spv = fopen(path, "r");
    if (spv == NULL)
    {
        printf(ERR SYS_DBG_PREFIX" Unable to read shader, exitting\n");
        exit(1);
    }
    fseek(spv, 0, SEEK_END);
    long size = ftell(spv);
    fseek(spv, 0, SEEK_SET);

    char buffer[size];
    fread(buffer, sizeof(buffer[0]), size, spv);
    fclose(spv);
    
    return buffer;
}
#endif