/*
 *      utility to make ram disk
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: ramdisk.c,v 1.1.1.1 2007/08/06 10:04:43 root Exp $
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int main(int argc, char** argv)
{
	int argNum=1;
	char script_file[80]={0}, buffer[200], outfile[200]={0};
	int block=-1, do_compress=1;
	FILE *fp;

	while (argNum < argc) {
		if ( !strcmp(argv[argNum], "-b") ) {
			if (++argNum >= argc)
				break;
			sscanf(argv[argNum], "%d", &block);
		}
		else if ( !strcmp(argv[argNum], "-s") ) {
			if (++argNum >= argc)
				break;
			sscanf(argv[argNum], "%s", script_file);
		}
		else if ( !strcmp(argv[argNum], "-o") ) {
			if (++argNum >= argc)
				break;
			sscanf(argv[argNum], "%s", outfile);
		}
		else if ( !strcmp(argv[argNum], "-n") ) {
			do_compress = 0;
		}
		argNum++;
	}

	if (block == -1 || !script_file[0] || !outfile[0] ) {
		printf("usage: buildfs [-n] -b block_size -s script_file -o target_file\n");
		printf("	-n : do not compress\n");
		exit(1);
	}

	fp = fopen(script_file, "r");
	if ( fp == NULL ) {
		printf("Invalid input file %s!\n", script_file);
		exit(1);
	}
	system("sync");

        sprintf(buffer, "dd if=/dev/zero of=/dev/ram bs=1k count=%d", block);
        system(buffer);

	sprintf(buffer, "mke2fs -c /dev/ram %d", block);
	system(buffer);
	system("mount -t ext2 /dev/ram /mnt");

	// execute script file
	while ( fgets(buffer, 100, fp) ) {
		int i=0;
		while (buffer[i] == ' ') i++;

		if (buffer[i] == '#')
			continue;
		if (buffer[i] =='\n')
			continue;
		system(&buffer[i]);
	}
	fclose(fp);

	// copy out ramdisk image and compress into a file
	system("sync");
	system("umount /dev/ram");
	sprintf(buffer, "dd if=/dev/ram bs=1k count=%d of=./ext2n", block);
	system(buffer);
	system("cp ext2n image");

	if (do_compress) {
		system("gzip -9v -f image");
		sprintf(buffer, "cp -f image.gz %s", outfile);
		system(buffer);
	}
	else {
		sprintf(buffer, "cp -f image %s", outfile);
		system(buffer);
	}

	unlink("./ext2n");
	unlink("./image.gz");
	unlink("./image");

	system("sync");
}
