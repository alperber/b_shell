#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <errno.h>

// komutun başı ve sonundaki boşlukları temizlemek için fonksiyonların prototipleri
char *ltrim(char *str, const char *seps);
char *rtrim(char *str, const char *seps);
char *trim(char *str, const char *seps);

// batch modunu fonksiyonu
void batch(int argc, char *argv[]);

// verilen stringin boş olup olmadığını kontrol etme
bool isStringSpace(char *cmd);


int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "Turkish");

    // birden fazla argüman verildiyse hata yazdır
    if (argc > 2)
    {
        fprintf(stdout, "Arguman sayisi cok fazla!!!\n");
        return -1;
    }
    else if (argc == 2)
    {
        batch(argc, argv);
    }
    else
    {
        bool isQuit = false;

        char *prompt = "prompt> ";
        char input[10000];
        char *quit = "quit";

        // önce komutlar noktalı virgül ile ayrıştırılır
        char div[] = ";";
        // her komut boşluğa göre argümanlarına ayrılır
        char div2[] = " ";

        // verilen input noktalı virgüle göre ayrılacak diziye yerleştirilecek
        char *komutlar[1000];

        while (!isQuit)
        {
            // prompt yazdırılır
            printf("%s", prompt);

            // komut satırını okur
            //gets(komut);
            fgets(input, 10000, stdin);

            // fgets enter'a basıldığından dolayı son karaktere \n atıyor onu düzeltmek için
            input[strlen(input) - 1] = '\0';

            // quit yazarsa çıkar
            if (!strcmp(trim(input, NULL), quit))
            {
                return 0;
            }

            // enter'a basarsa devam eder
            if (!strlen(input))
            {
                continue;
            }

            // karakter sayısı fazla ise hata yazısı ile devam eder
            if (strlen(input) > 512)
            {
                printf("Verilen komutun karakter sayısı 512'yi geçemez!!!\n");
                continue;
            }

            int komutSayisi = 0;

            komutlar[komutSayisi] = strtok(input, div);

            while (komutlar[komutSayisi] != NULL)
            {
                // sıradaki komut quit ise daha fazla okumayacak
                // ve ana döngüden çıkmak için isQuit true yapılır
                if (!strcmp(trim(komutlar[komutSayisi], NULL), quit))
                {
                    isQuit = true;
                    break;
                }
                if (isStringSpace(komutlar[komutSayisi]))
                {
                    komutlar[komutSayisi] = strtok(NULL, div);
                    continue;
                }
                komutSayisi++;
                komutlar[komutSayisi] = strtok(NULL, div);
            }

            char *arguments[komutSayisi][100];

            int i;
            for (i = 0; i < komutSayisi; i++)
            {
                // komut argümanlara parçalanır
                int j = 0;
                arguments[i][j] = strtok(komutlar[i], div2);

                while (arguments[i][j] != NULL)
                {
                    arguments[i][++j] = strtok(NULL, div2);
                }
            }


            // processler yaratılıp komutlar çalıştırılır
            for (i = 0; i < komutSayisi; i++)
            {
                pid_t pid = fork();
                if (pid < 0)
                {
                    printf("Child not created.\n");
                }
                else if (pid == 0)
                {
                    if (execvp(arguments[i][0], arguments[i]) == -1)
                    {
                        perror("Hata");
                    }
                    _exit(errno);
                    break;
                }
                // parent process tüm çocukları bekleyecek
                else
                {
                    while (wait(NULL) != -1 || errno != ECHILD)
                        ;
                }
            }
        }
    }

    return 0;
}

void batch(int argc, char *argv[])
{
    char *quit = "quit";

    // önce komutlar noktalı virgül ile ayrıştırılır
    char div[] = ";";
    // her komut boşluğa göre argümanlarına ayrılır
    char div2[] = " ";

    // dosya adı değişkene atanır
    char *fileName = argv[1];

    FILE *text = fopen(fileName, "r");

    // dosya açılmasıyla ilgili bir hata oldu
    if( text == NULL)
    {
        perror("Dosya Hatası ");
        _exit(errno);
    }

    bool isQuit = false;

    int satirSayisi = 0;
    while (!feof(text) && !isQuit)
    {
        // satır için tanımla
        char buffer[10000];

        // satırı buffer'a at
        fgets(buffer, 10000, text);


        // 512 karakter kontrolü
        if (strlen(buffer) > 512)
        {
            printf("%d. satır => Verilen satırın karakter sayısı 512'yi geçemez!!!\n", satirSayisi + 1);
            satirSayisi++;
            continue;
        }


        // fgets buffer'ın son karakterine \n atadığı için düzeltme yap
        // son satırın son karakterinde \n olmadığı için kontrol gerekti
        if(buffer[strlen(buffer) -1] == '\n')
        {
            buffer[strlen(buffer) - 1] = '\0';
        }

        // satir bos ise bu satiri calistirma
        if(isStringSpace(buffer))
        {
            continue;
        }

        // satırı çalıştırmadan önce ekrana yaz
        printf("%d. satır => %s\n", satirSayisi + 1, buffer);

        // verilen input noktalı virgüle göre ayrılacak diziye yerleştirilecek
        char *komutlar[1000];

        int komutSayisi = 0;

        komutlar[komutSayisi] = strtok(buffer, div);

        while (komutlar[komutSayisi] != NULL)
        {
            // sıradaki komut quit ise daha fazla okumayacak
            // ve ana döngüden çıkmak için isQuit true yapılır
            if (!strcmp(trim(komutlar[komutSayisi], NULL), quit))
            {
                printf("QUIT DETECTED\n");
                isQuit = true;
                break;
            }
            if (isStringSpace(komutlar[komutSayisi]))
            {
                komutlar[komutSayisi] = strtok(NULL, div);
                continue;
            }
            komutSayisi++;
            komutlar[komutSayisi] = strtok(NULL, div);
        }

        if(isQuit) break;

        satirSayisi++;

        // her komutun maksimum argüman sayısı max 100 olarak verildi
        char *arguments[komutSayisi][100];

        int i;
        for (i = 0; i < komutSayisi; i++)
        {
            // komut argümanlara parçalanır
            int j = 0;
            arguments[i][j] = strtok(komutlar[i], div2);

            while (arguments[i][j] != NULL)
            {
                arguments[i][++j] = strtok(NULL, div2);
            }
        }


        // processler yaratılıp komutlar çalıştırılır
        for (i = 0; i < komutSayisi; i++)
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                printf("Child not created.\n");
            }
            else if (pid == 0)
            {
                if (execvp(arguments[i][0], arguments[i]) == -1)
                {
                    //fprintf(stderr, "Bu komut hatalı => %s\n", komutlar[i]);
                    perror("Komut Hatası ");
                }
                _exit(errno);
                break;
            }
            // parent process tüm çocukları bekleyecek
            else
            {
                while (wait(NULL) != -1 || errno != ECHILD);
            }
        }
    }
    fclose(text);
}

char *ltrim(char *str, const char *seps)
{
    size_t totrim;
    if (seps == NULL)
    {
        seps = "\t\n\v\f\r ";
    }
    totrim = strspn(str, seps);
    if (totrim > 0)
    {
        size_t len = strlen(str);
        if (totrim == len)
        {
            str[0] = '\0';
        }
        else
        {
            memmove(str, str + totrim, len + 1 - totrim);
        }
    }
    return str;
}

char *rtrim(char *str, const char *seps)
{
    int i;
    if (seps == NULL)
    {
        seps = "\t\n\v\f\r ";
    }
    i = strlen(str) - 1;
    while (i >= 0 && strchr(seps, str[i]) != NULL)
    {
        str[i] = '\0';
        i--;
    }
    return str;
}

char *trim(char *str, const char *seps)
{
    return ltrim(rtrim(str, seps), seps);
}
bool isStringSpace(char *cmd)
{
    int i = 0;
    char ch = *cmd;
    while (ch != '\0')
    {
        if (!isspace(ch))
        {
            return false;
        }
        i++;
        ch = *(cmd + i);
    }
    return true;
}