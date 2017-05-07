#include <stdio.h>
#include <string.h>

int main (void)
{
   char name[80];

   printf("Hi! What's your name? ");
   fgets(name, sizeof(name), stdin);
   if (name[0])				// Remove trailing \n
      name[strlen(name)-1]='\0';
   printf("Hello, %s!\n", name);
   return 0;
}

