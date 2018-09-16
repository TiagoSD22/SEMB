/*
    Este código implementa a abertura de um arquivo de imagem em formato .PGM binário(P5) 
    Não são suportados arquivos em outros formatos, nem mesmo .pgm ASCII(P2).
    Também não suportadas imagens com resolução maior que 1080p (mais de 2073600 pixels)
    O objetivo principal é a implementação do algoritmo de Roberts para detecção de bordas e uma demonstração
    do seu uso para realce da imagem.

    @author Tiago Dionizio <tiagosdionizio@gmail.com>
*/

#include<math.h>
#include<stdio.h>
#include<stdio_ext.h>
#include<string.h>
#include<time.h>

typedef struct IMG_PGM{
    int largura,altura,valorMaximo;
    unsigned char pixels[2073600]; //vetor estático que suporta imagens de até 1080p (1920x1080)
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
                printf("\n\nImagem com resolução muito alta e não suportada!\n\n");   
                return 0;
            }
            fscanf(*img,"%d",&image->valorMaximo);
            if(image->valorMaximo > 255){
                printf("\n\nImagem com mais de 8 bits por pixel não é suportada!\n\n");
                return 0;
            }
        }
    }
    return 1;
}

int Ler_Pixels(FILE** img,ImagePGM* image){
    //esses whiles servem para pular possíveis comentários entre o final do cabeçalho do arquivo e os primeiros bytes de dados
    while (getc((*img)) == '#'){
        while (getc((*img)) != '\n');         
    }
    if (fread(image->pixels, sizeof(unsigned char), image->largura * image->altura, *img) != image->largura * image->altura) {
        //caso alguma dimensão do arquivo esteja diferente das informações do cabeçalho resultará em falha
        printf("\n\nArquivo com cabeçalho ou dados incoerentes!\n\n");
        fclose(*img);
        return 0;
    }
    printf("\n\nImagem carregada com sucesso!\nResolução: %dx%d\n\n",image->largura,image->altura); 
}

//Salva a imagem em um arquivo com o nome nomeArquivo passado como parâmetro em formato .pgm(P5)
void Escrever_Imagem(ImagePGM* image, char* nomeArquivo){
    FILE* saida;
    saida = fopen(nomeArquivo,"wb");
    fprintf(saida,"P5\n%d %d\n%d\n",image->largura,image->altura,image->valorMaximo);
    fwrite(image->pixels,sizeof(unsigned char),(image->largura * image->altura),saida);
    fprintf(saida,"\r");
    printf("Arquivo %s gerado com sucesso!\n\n",nomeArquivo);
    fclose(saida);
}

//função para manter o valor do pixel entre 0 e 255
void Truncar_Pixel(int* pixel){
    if(*pixel > 255) *pixel = 255;
    if(*pixel < 0)   *pixel = 0;
}

//Filtro de Roberts para detecção de bordas na imagem 
void Roberts_Cross(ImagePGM* image){
    int i,j, intensidadePixel;
    //cria um array bidimensional auxiliar para manipulação mais fácil do array de pixels no struct ImagePGM
    unsigned char image_aux[image->altura + 1][image->largura + 1];
    for(i = 0; i < image->altura + 1; i++){
        for(j = 0; j < image->largura + 1; j++){
            if(i < image->altura && j < image->largura)
                image_aux[i][j] = image->pixels[(image->largura * i) + j];
            else
                image_aux[i][j] = 0;
        }
    }

    image->valorMaximo = 0;//O valor da maior intensidade provavelmente será alterado, este valor será gravado no arquivo de saída

    /*
        Pela definição do algoritmo de Roberts, para um pixel em x,y dado por P(x,y) = p, seu novo valor, p' dado por P'(x,y) será:
        Gx = (P(x,y) - P(x + 1, y + 1))
        Gy = (P(x, y + 1) - P(x + 1, y))
        P'(x,y) = sqrt(Gx^2 + Gy^2)
    */
    for(i = 0; i < image->altura; i++){
        for(j = 0; j < image->largura; j++){
            intensidadePixel = (int)(sqrt((int)(pow((int)(image_aux[i][j] - image_aux[i + 1][j + 1]),2)) + (int)(pow((int)(image_aux[i][j + 1] - image_aux[i + 1][j]),2))));
            Truncar_Pixel(&intensidadePixel);//caso algum pixel p' fique acima de 255
            //guarda o novo valor maximo de intensidade para ser gravado no arquivo de saida
            if(intensidadePixel > image->valorMaximo) image->valorMaximo = intensidadePixel;
            image_aux[i][j] = intensidadePixel;
            //copia o novo pixel do array auxiliar para o original
            image->pixels[(image->largura * i) + j] = (unsigned char)(image_aux[i][j]);
        }
    }
}

//Função usada para combinar a imagem original com a saída do Roberts para realce das bordas
void Somar_Imagens(ImagePGM* img1, ImagePGM* img2){
    int i,j;
    int novaIntensidade;
    for(i = 0; i < (img1->altura * img1->largura); i++){
        novaIntensidade = img1->pixels[i] + img2->pixels[i]; 
        Truncar_Pixel(&novaIntensidade);
        img1->pixels[i] = novaIntensidade;
    }
}

void Registrar_Tempo(char* nome_imagem, double* tempo){
    FILE* arquivo = fopen("Tempos.txt","a");
    fprintf(arquivo,"\n%s %lf",nome_imagem,*tempo);
}

int main(int narg, char* argv[]){
    FILE* img;
    ImagePGM image;
    ImagePGM bordas;
    char nome_arquivo[50];

    //caso o código seja executado passando um arquivo de imagem como parâmetro
    if(narg > 1) strcpy(nome_arquivo,argv[1]);
    else{
        printf("Digite o nome do arquivo (.pgm) que deseja abrir: ");
        __fpurge(stdin);
        fgets(nome_arquivo,50,stdin);
        if(nome_arquivo[strlen(nome_arquivo) - 1] == '\n')
            nome_arquivo[strlen(nome_arquivo) - 1] = '\0';
    }
    
    if(Abrir_Imagem(&img,&image,nome_arquivo)){
        clock_t clock_ticks;
        double tempo;
        char nome_saida_Robert[30];
        char nome_saida_Realce[30];
        strcpy(nome_saida_Robert,nome_arquivo);
        nome_saida_Robert[strlen(nome_saida_Robert) - 4] = '\0';
        strcpy(nome_saida_Realce,nome_saida_Robert);
        strcat(nome_saida_Robert,"_Robert.PGM");
        strcat(nome_saida_Realce,"_Realce.PGM");
        Ler_Pixels(&img,&image);
        bordas = image;
        clock_ticks = clock();
        Roberts_Cross(&bordas);
        clock_ticks = clock() - clock_ticks;
        tempo = ((double)clock_ticks)/CLOCKS_PER_SEC;
        Registrar_Tempo(nome_arquivo,&tempo);
        //guarda o resultado das bordas de Robert para verificar se o algoritmo encontrou algo coerente
        Escrever_Imagem(&bordas,nome_saida_Robert);
        Somar_Imagens(&image, &bordas);//cria a imagem com bordas realçadas e a salva em seguida para comparar com a original
        Escrever_Imagem(&image,nome_saida_Realce);
    }

    fclose(img);
    return 0;
}
