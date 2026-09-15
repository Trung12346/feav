#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "debugger.h"
#include "windows_utils/file.h"
#include "main.h"
#include <inttypes.h>
#include "notmine/u16_to_u8.h"
#include "notmine/u32_to_u8.h"
#include "string_utils.h"

#ifndef ARG_VAL
#define ARG_VAL char*value=argv[i+1];if(strncmp(value,"--", 2) == 0){printf(ERR SYS_DBG_PREFIX" Invalid argument for %s",argv[i]);exit(1);}
#endif

typedef enum {
    UTF8,
    UTF16,
    UTF32,
    ASCII
} Encoding;

Encoding preproc_encode = ASCII;
char *preproc_output_html_filename = NULL;

void preprocess(int argc, char **argv, char **dst, uint64_t *size)
{
    if (argc == 1);
    else
    {
        for (uint32_t i = 0U; i < argc; i++)
        {
            if (strcmp(argv[i], "--encode") == 0 && i + 1 < argc)
            {
                ARG_VAL
                if (strcmp(value, "ASCII") == 0) preproc_encode = ASCII;
                else if (strcmp(value, "UTF-8") == 0) preproc_encode = UTF8;
                else if (strcmp(value, "UTF-16") == 0) preproc_encode = UTF16;
                else if (strcmp(value, "UTF-32") == 0) preproc_encode = UTF32;
                
            } else
            if (strcmp(argv[i], "--out") == 0 && i + 1 < argc)
            {
                ARG_VAL
                preproc_output_html_filename = value;
            }
        }
    }

    uint64_t file_size;
    char *file;
    file_read("index.html", &file_size, NULL);
    file = malloc((size_t)file_size);
    file_read("index.html", &file_size, file);

    switch (preproc_encode)
    {
        case UTF16:
            uint16_t *p16 = (uint16_t *)file;
            file = u16_to_u8(p16, file_size, NULL, &file_size);
            break;
        case UTF32:
            uint32_t *p32 = (uint32_t *)file;
            file = u32_to_u8(p32, file_size, NULL, &file_size);
            break;
        default:
    }

    printf("%u" PRIu64 "\n", file_size);

    char *file_builder_buffer;
    uint64_t builder_size;
    char *file_ext_buffer;
    uint64_t ext_size;

    for (;;)
    {
        char *p_strt = strstr(file, "<!--");

        if (p_strt == NULL) break;

        uint64_t index = p_strt - file;
        char *p_end = strstr(file + index, "-->");
        if (p_end == NULL)
        {
            p_end = file + (file_size - 1);
        }
        uint64_t end_index = p_end - file;

        strtrim(&file, &file_size, index, end_index + 2);
    }
    for (;;)
    {
        char *p = strstr(file, "#include");

        if (p == NULL) break;
        uint64_t index = p - file;
        uint64_t start_index = index + strlen("#include");
        uint64_t value_start = 0;
        uint64_t value_end = 0;
        bool value_recording = false;
        bool commenting = false;
        uint32_t value_size = 0;
        char *value;

        printf("%u\n", index);
        printf("%u\n", start_index);

        for (uint64_t i = start_index;; i++)
        {
            char current_char = file[i];
            printf("%c\n", current_char);
            if (current_char == '\n')
            {
                printf(ERR PREPROC_DBG_PREFIX" Unexpected byte at index %u", i);
                exit(1);
            } else if (current_char == '\0')
            {
                printf(ERR PREPROC_DBG_PREFIX" Unexpected byte at index %u", i);
                exit(1);
            } else if ((current_char == '<' || current_char == '"') && !value_recording)
            {
                value_recording = true;
                value_start = i + 1;
            } else if ((current_char == '>' || current_char == '"') && value_recording)
            {
                value_end = i - 1;
                value_size = value_end - value_start + 1;
                value = malloc(value_size);
                memcpy(value, file + value_start, value_size);

                printf("%s\n", value);
                
                strtrim(&file, &file_size, index, value_end + 1);

                fwrite(file, 1, file_size, stdout);
                break;
            }
        }

        file_read(value, &ext_size, NULL);
        file_ext_buffer = malloc(ext_size);
        file_read(value, &ext_size, file_ext_buffer);
        fwrite(file_ext_buffer, 1, ext_size, stdout);

        builder_size = file_size + ext_size;
        printf("%u\n", builder_size);
        file_builder_buffer = malloc(builder_size);
        // memcpy(file_builder_buffer, file_ext_buffer, ext_size);
        // memcpy(file_builder_buffer + ext_size, file, file_size);
        memcpy(file_builder_buffer, file, file_size);
        
        uint64_t move_size = file_size - index;
        memmove(file_builder_buffer + index + ext_size, file_builder_buffer + index, move_size);
        memcpy(file_builder_buffer + index, file_ext_buffer, ext_size);

        free(file);
        file_size = builder_size;
        file = malloc(file_size);
        memcpy(file, file_builder_buffer, file_size);

        free(file_builder_buffer);
        free(file_ext_buffer);
        file_builder_buffer = NULL;
        file_ext_buffer = NULL;
        builder_size = 0;
        ext_size = 0;
    }

    if (preproc_output_html_filename != NULL)
    {
        create_write_file(preproc_output_html_filename, file, file_size);
        free(preproc_output_html_filename);
    }

    *dst = malloc(file_size);
    memcpy(*dst, file, file_size);
    *size = file_size;
}

#endif