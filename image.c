/*
    Este código implementa a abertura de um arquivo de imagem em formato .PGM binário(P5) 
    Não são suportados arquivos em outros formatos, nem mesmo .pgm ASCII(P2).
    Também não são suportadas imagens com resolução maior que 1080p (mais de 2073600 pixels)
    O objetivo principal é a implementação do algoritmo de Roberts para detecção de bordas e uma demonstração
    do seu uso para realce da imagem.

    @author Tiago Dionizio <tiagosdionizio@gmail.com>
*/

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

//Salva a imagem em um arquivo com o nome nomeArquivo passado como parâmetro em formato .pgm(P5)
void Escrever_Imagem(ImagePGM* image, char* nomeArquivo){
    int i,j;
    FILE* saida;
    saida = fopen(nomeArquivo,"wb");
    fprintf(saida,"P5\n%d %d\n%d\n",image->largura,image->altura,image->valorMaximo);
    for(i = 0; i < image->altura; i++){
        for(j = 0; j < image->largura; j++)
            putc(image->pixels[i][j],saida);
    }
    fprintf(saida,"\n\r");
    printf("Arquivo %s gerado com sucesso!\n\n",nomeArquivo);
    fclose(saida);
}

//função para manter o valor do pixel entre 0 e 255
void Truncar_Pixel(int* pixel){
    if(*pixel > 255) *pixel = 255;
    if(*pixel < 0)   *pixel = 0;
}

//Técnica de padding para trabalhar com o filtro de Roberts
ImagePGM Padding(ImagePGM* img){
    int i,j;
    ImagePGM imgPadd;
    imgPadd.altura = img->altura + 2;
    imgPadd.largura = img->largura + 2;
    imgPadd.valorMaximo = img->valorMaximo;
    for(i = 0; i < imgPadd.altura; i++){
        for(j = 0; j < imgPadd.largura; j++){
            imgPadd.pixels[i][j] =  0;
        }
    }
    for(i = 0; i < img->altura; i++){
        for(j = 0; j < img->largura; j++){
            imgPadd.pixels[i + 1][j + 1] = img->pixels[i][j];
        }
    }
    return imgPadd;
}

//Filtro de Roberts para detecção de bordas na imagem 
void Roberts_Cross_Padding(ImagePGM* img){
    int i,j,x,y,novaIntensidade,gx,gy;    
    int robertX[2][2] = {{1,0},{0,-1}};
    int robertY[2][2] = {{0,1},{-1,0}};
    ImagePGM imgPadd = Padding(img);
    for(i = 0; i < img->altura; i++){
        for(j = 0; j < img->largura; j++){
            gx = 0;
            gy = 0;
            for(x = 0; x < 2; x++){
                for(y = 0; y < 2; y++){
                    if(i > 0 && j > 0){
                        gx += ((int)(imgPadd.pixels[i + x][j + y])) * robertX[x][y];
                        gy += ((int)(imgPadd.pixels[i + x][j + y])) * robertY[x][y];
                    }
                    else{
                        gx += ((int)(img->pixels[i + x][j + y])) * robertX[x][y];
                        gy += ((int)(img->pixels[i + x][j + y])) * robertY[x][y];
                    }
                }
            }
            novaIntensidade = (int)sqrt(pow((double)gx,2) + pow((double)gy,2));
            Truncar_Pixel(&novaIntensidade);
            img->pixels[i][j] = (unsigned char)novaIntensidade;
        }
    }
}

//Função usada para combinar a imagem original com a saída do Roberts para realce das bordas
void Somar_Imagens(ImagePGM* img1, ImagePGM* img2){
    int i,j;
    int novaIntensidade;
    for(i = 0; i < img1->altura; i++){
        for(j = 0; j < img1->largura; j++){
            novaIntensidade = img1->pixels[i][j] + img2->pixels[i][j]; 
            Truncar_Pixel(&novaIntensidade);
            img1->pixels[i][j] = novaIntensidade;
        }
    }
}

int main(int narg, char* argv[]){
    FILE* img;
    ImagePGM image;
    ImagePGM bordas;
    char nome_arquivo[50];
    //caso o código seja executado passando um arquivo de imagem como parâmetro
    if(narg > 1) 
        strcpy(nome_arquivo,argv[1]);
    else{
        printf("Digite o nome do arquivo (.pgm) que deseja abrir: ");
        __fpurge(stdin);
        fgets(nome_arquivo,50,stdin);
        if(nome_arquivo[strlen(nome_arquivo) - 1] == '\n')
            nome_arquivo[strlen(nome_arquivo) - 1] = '\0';
    }
    
    if(Abrir_Imagem(&img,&image,nome_arquivo)){
        char nome_saida_Robert[30];
        char nome_saida_Realce[30];
        strcpy(nome_saida_Robert,nome_arquivo);
        nome_saida_Robert[strlen(nome_saida_Robert) - 4] = '\0';
        strcpy(nome_saida_Realce,nome_saida_Robert);
        strcat(nome_saida_Robert,"_Robert.PGM");
        strcat(nome_saida_Realce,"_Realce.PGM");
        Ler_Pixels(&img,&image);
        bordas = image;
        Roberts_Cross_Padding(&bordas);
        //guarda o resultado das bordas de Robert para verificar se o algoritmo encontrou algo coerente
        Escrever_Imagem(&bordas,nome_saida_Robert);
        Somar_Imagens(&image, &bordas);//cria a imagem com bordas realçadas e a salva em seguida para comparar com a original
        Escrever_Imagem(&image,nome_saida_Realce);
    }

    fclose(img);
    return 0;
}
