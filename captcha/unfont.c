#include <stdio.h>
#include "font.h"

void copy(int s, int e, int idx) {
	int r,i;
	for(i=s;i<e;i++) {
		for(r=0;r<50;r++) {
			if(im[r][i]!=' ') goto findend;
		}
	}

	findend: s=i;

	for(i=e;i>s;i--) {
		for(r=0;r<50;r++) {
			if(im[r][i]!=' ') goto go;
		}
	}

	go: e=i+1;

	printf("static int8_t lt%u[]={",idx);

	for(r=0;r<50;r++) {
		int l=0;
		for(i=s;i<e;i++) {
			char c=im[r][i];
			if(c==' ') { l++; }
			else {
				if(l) { printf("-%u,",l); l=0; }
				printf("%u,",colors[c-'a']);
			}
		}
		printf("-100,");
	}
	printf("-101};\n");

}

int main() {
	int i,st=0,idx=0;
	for(i=0;i<width;i++) {
		if(im[0][i]=='a') {
			copy(st+1,i-1,idx);
			idx++;
			st=i;
		}
	}

	printf("static int8_t *lt[]={");
	for(i=0;i<idx;i++) {
		printf("lt%d,",(i==6||i==4)?0:i);
	}
	printf("};\n\n");
	return 0;
}


