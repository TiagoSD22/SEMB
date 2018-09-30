#include<math.h>
#include<stdio.h>
#include<stdio_ext.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

typedef struct IMG_PGM{
    int largura,altura,valorMaximo;
    unsigned char pixels[1080][1920];//vetor estático que suporta imagens com resolução de até 1080p
}ImagePGM;

/*
    A função Abrir_Imagem recebe como argumentos um ponteiro para arquivo em que ficará carregada a imagem
    e um ponteiro para o struct ImagePGM que conterá as informações binárias do arquivo. Caso a imagem a ser aberta não exista,
    ou não esteja no formato .pgm (P5), ou tenha resolução maior que 1080p, ou tenha mais de 8 bits por pixel, resultará em falha!
*/
int Abrir_Imagem(FILE** img, ImagePGM* image, char* nome_arquivo){
    *img = fopen(nome_arquivo,"rb");
    if(!(*img)){
        printf("\n\nArquivo não encontrado!\n\n");
        return 0;
    }
    else{
        char tipo[3];
        fgets(tipo,3,*img);
        if(strcmp(tipo,"P5") != 0){
            printf("\n\nArquivo não está no formato .PGM!\n\n");
            return 0;
        }
        else{
            fscanf(*img,"%d %d",&image->largura,&image->altura);
            if(image->largura * image->altura > 2073600) {
                printf("\n\nImagens com resolução maior do que 1080p não são suportadas!\n\n");   
                return 0;
            }
            fscanf(*img,"%d",&image->valorMaximo);
            if(image->valorMaximo > 255){
                printf("\n\nImagens com mais de 8 bits por pixel não são suportadas!\n\n");
                return 0;
            }
        }
    }
    return 1;
}

void Ler_Pixels(FILE** img,ImagePGM* image){
    int i,j;
    //esses whiles servem para pular possíveis comentários entre o final do cabeçalho do arquivo e os primeiros bytes de dados
    while (getc((*img)) == '#'){
        while (getc((*img)) != '\n');         
    }
    for(i = 0; i < image->altura; i++){
        for(j = 0; j < image->largura; j++){
            image->pixels[i][j] = getc(*img);
        }
    } 
    printf("\n\nImagem carregada com sucesso!\nResolução: %dx%d\n\n",image->largura,image->altura); 
}

void Comparar_Imagens(char* n_img1, char* n_img2){
    FILE* img1_file;
    FILE* img2_file;
    ImagePGM img1;
    ImagePGM img2;
    int i, j, acerto = 0;
    double porcentagem;
    char nome_img1[50], nome_img2[50];
    if(strlen(n_img1) > 0 && strlen(n_img2) > 0){
    	strcpy(nome_img1,n_img1);
	strcpy(nome_img2,n_img2);
    }
    else{

    printf("Digite o nome do primeiro arquivo (.pgm) a ser comparado: ");
    __fpurge(stdin);
    fgets(nome_img1,50,stdin);
    if(nome_img1[strlen(nome_img1) - 1] == '\n')
        nome_img1[strlen(nome_img1) - 1] = '\0';
    printf("Digite o nome do segundo arquivo (.pgm) a ser comparado: ");
    __fpurge(stdin);
    fgets(nome_img2,50,stdin);
    if(nome_img2[strlen(nome_img2) - 1] == '\n')
        nome_img2[strlen(nome_img2) - 1] = '\0';

    }

    if(Abrir_Imagem(&img1_file, &img1, nome_img1) && Abrir_Imagem(&img2_file, &img2, nome_img2)){
        Ler_Pixels(&img1_file,&img1);
        Ler_Pixels(&img2_file,&img2);
    }

    for(i = 0; i < img1.altura; i++){
        for(j = 0; j < img1.largura; j++){
            if(img1.pixels[i][j] == img2.pixels[i][j]){
                acerto++;
            }
        }
    }

    porcentagem = (acerto / ((float)img1.altura * (float)img1.largura)) * 100;

    printf("As imagens são %f %% identicas. \n\n",porcentagem);
}
int main(int narg, char* argv[]){
    if(narg == 3)
	Comparar_Imagens(argv[1],argv[2]);
    else
    	Comparar_Imagens("","");
    return 0;
}