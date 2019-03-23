#ifndef _FILE_TRANSFER_H
#define _FILE_TRANSFER_H

int begin_file_upload(int client, char * filename);
int begin_file_download(int client, char * ip, char * filename, char * path);

#endif
