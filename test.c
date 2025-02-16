#include <stdio.h>
#include <string.h>

typedef char trckr_text_small[16];

struct mystruct {
	int a;
	int b;
	trckr_text_small text;
};


void stringasdf(trckr_text_small input)
{
	printf("%d\n", sizeof(trckr_text_small));
	// strcpy(input->text, NULL);
}

int main()
{
	trckr_text_small test;
	stringasdf(test);
}
