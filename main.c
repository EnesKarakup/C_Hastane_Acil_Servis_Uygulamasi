#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_HL7_MESSAGE 1000   // HL7 mesajı için maksimum karakter sayısı

int hastaNumarasiCounter = 0;  // Gün içinde ki hasta sayısını tutan global sayaç

// Hasta bilgilerini tutan yapı
typedef struct Hasta {
    char tcKimlikNo[20];
    char hastaNumarasi[1000];// hastaNumarasi: sistemin otomatik oluşturduğu gün içinde ki sıra numarası
    char ad[50];
    char soyad[50];
    char dogumTarihi[12];    // "YYYY-MM-DD"
    char cinsiyet;
    char adres[500];
    char telefon[15];
    int OncelikPuani;        // Triyaj sonucunda hesaplanan puan
} Hasta;

// Triyaj bilgilerini tutan yapı
// aciliyetSeviyesi: 1 (en acil) ile 5 (en az acil) arasında
typedef struct TriyajBilgisi {
    int aciliyetSeviyesi;
    char sikayet[500];
    char hayatiTehlikesi;   // 'E' ya da 'H'
    float atesDegeri;
    char gelisTarihi[12];
    char gelisSaati[10];
    char color[10];        // Belirlenen aciliyet rengi
} TriyajBilgisi;

// ilaç bilgilerini tutan yapı
typedef struct Ilac {
    char ilacAdi[50];
    char doz[20];
    char kullanimSekli[100];
    int adet;
} Ilac;

// Reçete bilgilerini tutan yapı
// hastaId: TC Kimlik No kullanılarak hasta tanımlanır
typedef struct Recete {
    char receteNo[15];
    char hastaId[20];
    char doktorId[20];
    char tarih[11];
    Ilac* ilaclar;
    int ilacSayisi;
} Recete;

// Hasta düğümü (bağlı liste)
// Hasta bilgisi ve ilgili triyaj bilgilerini içerir
typedef struct HastaNode {
    Hasta hastaBilgisi;
    TriyajBilgisi triyajBilgisi;
    struct HastaNode* next;
} HastaNode;

// Hasta kuyruğu(Queue)
typedef struct HastaQueue {
    HastaNode* front;  // Kuyruğun başındaki hasta
    HastaNode* rear;   // Kuyruğun sonundaki hasta
    int size;
} HastaQueue;

// Reçete düğümü (stack için)
typedef struct ReceteNode {
    Recete receteBilgisi;
    struct ReceteNode* next;
} ReceteNode;

// Reçete stack (yığın) yapısı
typedef struct ReceteStack {
    ReceteNode* top;   // Yığının en üstündeki reçete
    int size;
} ReceteStack;

// HL7 mesajı için yapı (raw ve bölümlere ayrılmış format)
typedef union HL7Message {
    char rawHL7[MAX_HL7_MESSAGE];
    struct {
        char messageHeader[100];
        char patientInfo[200];
        char observationInfo[200];
        char medicationInfo[300];
        char messageFooter[100];
    } hl7Format;
} HL7Message;

// Fonksiyon prototipleri
void TriyajBelirleme(TriyajBilgisi* triyaj);
void hastaQueueInit(HastaQueue* queue);
void receteStackInit(ReceteStack* stack);
void hastaKabul(HastaQueue* queue, Hasta yeniHasta, TriyajBilgisi triyaj);
Hasta* hastaGetir(HastaQueue* queue);
void hastaSil(HastaQueue* queue);
char* receteNoUret();
void receteOlustur(ReceteStack* stack, Hasta hasta, char* doktorId, Ilac* ilaclar, int ilacSayisi);
void receteYazdir(ReceteStack* stack);
HL7Message hastaToHL7(Hasta hasta, TriyajBilgisi triyaj);
void hl7MesajGonder(HL7Message mesaj);
Hasta hl7ToHasta(HL7Message mesaj);
void tumHastalariListele(HastaQueue* queue);
void menuGoster(HastaQueue* hastaQueue, ReceteStack* receteStack);
void freeHastaQueue(HastaQueue* queue);
void freeReceteStack(ReceteStack* stack);

// --- Fonksiyonlar ---

//  TriyajBelirleme: Şikayet, hayati tehlike ve ateş değerine göre aciliyet seviyesi belirler
void TriyajBelirleme(TriyajBilgisi* triyaj) {
    if(triyaj->hayatiTehlikesi == 'E' || triyaj->hayatiTehlikesi == 'e') {
        triyaj->aciliyetSeviyesi = 1;
        strcpy(triyaj->color, "Kirmizi");
        return;
    }
    if (strstr(triyaj->sikayet, "sok belirtileri") != NULL ||
        strstr(triyaj->sikayet, "bilinc kaybi") != NULL ||
        strstr(triyaj->sikayet, "ciddi kafa travmasi") != NULL ||
        strstr(triyaj->sikayet, "ani felc") != NULL ||
        strstr(triyaj->sikayet, "kalp durmasi") != NULL ||
        strstr(triyaj->sikayet, "masif kanama") != NULL ||
        strstr(triyaj->sikayet, "ciddi zehirlenme") != NULL ||
        strstr(triyaj->sikayet, "uzun suren nobet") != NULL ||
        strstr(triyaj->sikayet, "ciddi solunum sikintisi") != NULL ||
        strstr(triyaj->sikayet, "anafilaktik sok") != NULL ||
        strstr(triyaj->sikayet, "ileri derece yanik") != NULL ||
        strstr(triyaj->sikayet, "komada") != NULL) {
        triyaj->aciliyetSeviyesi = 1;
        strcpy(triyaj->color, "Kirmizi");
    }
    else if(strstr(triyaj->sikayet, "yogun kanama") != NULL ||
            strstr(triyaj->sikayet, "siddetli karin agrisi") != NULL ||
            strstr(triyaj->sikayet, "gorme kaybi") != NULL ||
            strstr(triyaj->sikayet, "siddetli yaniklar") != NULL ||
            strstr(triyaj->sikayet, "kontrolsuz kusma") != NULL ||
            strstr(triyaj->sikayet, "siddetli agri") != NULL ||
            strstr(triyaj->sikayet, "ciddi dehidratasyon") != NULL ||
            strstr(triyaj->sikayet, "akut konfuzyon") != NULL ||
            strstr(triyaj->sikayet, "gogus agrisi") != NULL ||
            strstr(triyaj->sikayet, "aktif kanama") != NULL ||
            strstr(triyaj->sikayet, "orta derece solunum zorluğu") != NULL) {
        triyaj->aciliyetSeviyesi = 2;
        strcpy(triyaj->color, "Turuncu");
    }
    else if(strstr(triyaj->sikayet, "orta siddette agri") != NULL ||
            strstr(triyaj->sikayet, "kirik suphesi") != NULL ||
            strstr(triyaj->sikayet, "hafif yanik") != NULL ||
            strstr(triyaj->sikayet, "orta siddette enfeksiyon") != NULL ||
            strstr(triyaj->sikayet, "tekrarlayan kusma") != NULL ||
            strstr(triyaj->sikayet, "siddetli bas agrisi") != NULL ||
            strstr(triyaj->sikayet, "orta derece dehidratasyon") != NULL ||
            strstr(triyaj->sikayet, "uzuv yaralanmasi") != NULL ||
            strstr(triyaj->sikayet, "kontrol altina alinmis kanama") != NULL) {
        triyaj->aciliyetSeviyesi = 3;
        strcpy(triyaj->color, "Sari");
    }
    else if(strstr(triyaj->sikayet, "hafif burkulma") != NULL ||
            strstr(triyaj->sikayet, "soguk alginligi") != NULL ||
            strstr(triyaj->sikayet, "hafif kas agrisi") != NULL ||
            strstr(triyaj->sikayet, "kusma olmadan bulanti") != NULL ||
            strstr(triyaj->sikayet, "hafif ates") != NULL ||
            strstr(triyaj->sikayet, "kucuk kesik") != NULL ||
            strstr(triyaj->sikayet, "hafif bas agrisi") != NULL ||
            strstr(triyaj->sikayet, "hafif cilt enfeksiyonu") != NULL) {
        triyaj->aciliyetSeviyesi = 4;
        strcpy(triyaj->color, "Yesil");
    }
    else {
        triyaj->aciliyetSeviyesi = 5;
        strcpy(triyaj->color, "Mavi");
    }
    if(triyaj->atesDegeri > 39.0 && triyaj->aciliyetSeviyesi > 1) { // Ateş 39°C üzerindeyse, aciliyet bir kademe yükselir
        triyaj->aciliyetSeviyesi--;
        triyaj->aciliyetSeviyesi--;
        switch(triyaj->aciliyetSeviyesi) {
            case 1: strcpy(triyaj->color, "Kirmizi"); break;
            case 2: strcpy(triyaj->color, "Turuncu"); break;
            case 3: strcpy(triyaj->color, "Sari"); break;
            case 4: strcpy(triyaj->color, "Yesil"); break;
            case 5: strcpy(triyaj->color, "Mavi"); break;
        }
    }
}

// hastaQueueInit: Hasta kuyruğunu başlatır
void hastaQueueInit(HastaQueue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

// receteStackInit: Reçete stack'ini başlatır
void receteStackInit(ReceteStack* stack) {
    stack->top = NULL;
    stack->size = 0;
}

// hastaKabul: Yeni hastayı kuyruğa ekler, triyaj sonucu öncelik puanı hesaplanır
void hastaKabul(HastaQueue* queue, Hasta yeniHasta, TriyajBilgisi triyaj) {
    TriyajBelirleme(&triyaj);                   // Triyaj bilgilerine göre aciliyet seviyesi belirlenir
    // öncelikli hastalara yüksek puan verilmesini sağlamak için gerekli olan formül:
    yeniHasta.OncelikPuani = (6 - triyaj.aciliyetSeviyesi) * 20; //Öncelik Puanı 100 üzerinden değerlendirilmektedir

    // Yeni hasta düğümü oluşturulur
    HastaNode* yeniNode = (HastaNode*)malloc(sizeof(HastaNode));
    if (yeniNode == NULL) {
        fprintf(stderr, "Bellek tahsisi basarisiz!\n");
        exit(EXIT_FAILURE);
    }
    yeniNode->hastaBilgisi = yeniHasta;
    yeniNode->triyajBilgisi = triyaj;
    yeniNode->next = NULL;

    //öncelik puanlarına göre azalan sırada (yüksek puanlı hastalar önce) sıralıyor.
    if(queue->size == 0) {
        queue->front = yeniNode;
        queue->rear = yeniNode;
    } else if(yeniHasta.OncelikPuani > queue->front->hastaBilgisi.OncelikPuani) {
        yeniNode->next = queue->front;
        queue->front = yeniNode;
    } else {
        HastaNode* temp = queue->front;
        while(temp->next != NULL && yeniHasta.OncelikPuani <= temp->next->hastaBilgisi.OncelikPuani) {
            temp = temp->next;
        }
        yeniNode->next = temp->next;
        temp->next = yeniNode;
        if(yeniNode->next == NULL)
            queue->rear = yeniNode;
    }
    queue->size++;
    printf("Hasta kabul edildi.\nTC Kimlik No: %s, Hasta Numarasi: %s, Oncelik Puani: %d, Renk: %s\n",
           yeniHasta.tcKimlikNo, yeniHasta.hastaNumarasi, yeniHasta.OncelikPuani, triyaj.color);
}

// hastaGetir: Kuyruğun başındaki hastayı getirir
Hasta* hastaGetir(HastaQueue* queue) {
    if(queue->size == 0) {
        printf("Sirada hasta yok!\n");
        return NULL;
    }
    return &(queue->front->hastaBilgisi);
}

// hastaSil: Muayene sonrası kuyruğun başındaki hastayı siler
void hastaSil(HastaQueue* queue) {
    if(queue->size == 0) {
        printf("Sirada hasta yok!\n");
        return;
    }
    HastaNode* temp = queue->front;
    queue->front = queue->front->next;
    if(queue->front == NULL)
        queue->rear = NULL;
    free(temp);
    queue->size--;
    printf("Hasta muayenesi tamamlandi ve siradan cikartildi.\n");
}

// receteNoUret: Otomatik reçete numarası üretir
char* receteNoUret() {
    static char receteNo[15];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    sprintf(receteNo, "R%04d%02d%02d%04d",
            tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday, rand() % 10000);
    return receteNo;
}

// receteOlustur: Reçete oluşturup reçete stack'ine ekler
void receteOlustur(ReceteStack* stack, Hasta hasta, char* doktorId, Ilac* ilaclar, int ilacSayisi) {
    ReceteNode* yeniNode = (ReceteNode*)malloc(sizeof(ReceteNode));
    if(yeniNode == NULL) {
        fprintf(stderr, "ReceteNode bellek tahsisi basarisiz!\n");
        exit(EXIT_FAILURE);
    }
    strncpy(yeniNode->receteBilgisi.receteNo, receteNoUret(), sizeof(yeniNode->receteBilgisi.receteNo) - 1);
    yeniNode->receteBilgisi.receteNo[sizeof(yeniNode->receteBilgisi.receteNo) - 1] = '\0';
    strncpy(yeniNode->receteBilgisi.hastaId, hasta.tcKimlikNo, sizeof(yeniNode->receteBilgisi.hastaId) - 1);
    yeniNode->receteBilgisi.hastaId[sizeof(yeniNode->receteBilgisi.hastaId) - 1] = '\0';
    strncpy(yeniNode->receteBilgisi.doktorId, doktorId, sizeof(yeniNode->receteBilgisi.doktorId) - 1);
    yeniNode->receteBilgisi.doktorId[sizeof(yeniNode->receteBilgisi.doktorId) - 1] = '\0';
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    snprintf(yeniNode->receteBilgisi.tarih, sizeof(yeniNode->receteBilgisi.tarih),
             "%04d-%02d-%02d", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);
    yeniNode->receteBilgisi.ilaclar = (Ilac*)malloc(sizeof(Ilac) * ilacSayisi);
    if(yeniNode->receteBilgisi.ilaclar == NULL) {
        fprintf(stderr, "Ilac dizisi bellek tahsisi basarisiz!\n");
        free(yeniNode);
        exit(EXIT_FAILURE);
    }
    yeniNode->receteBilgisi.ilacSayisi = ilacSayisi;
    int i=0;
    for(i = 0; i < ilacSayisi; i++) {
        strncpy(yeniNode->receteBilgisi.ilaclar[i].ilacAdi, ilaclar[i].ilacAdi, sizeof(yeniNode->receteBilgisi.ilaclar[i].ilacAdi)-1);
        yeniNode->receteBilgisi.ilaclar[i].ilacAdi[sizeof(yeniNode->receteBilgisi.ilaclar[i].ilacAdi)-1] = '\0';
        strncpy(yeniNode->receteBilgisi.ilaclar[i].doz, ilaclar[i].doz, sizeof(yeniNode->receteBilgisi.ilaclar[i].doz)-1);
        yeniNode->receteBilgisi.ilaclar[i].doz[sizeof(yeniNode->receteBilgisi.ilaclar[i].doz)-1] = '\0';
        strncpy(yeniNode->receteBilgisi.ilaclar[i].kullanimSekli, ilaclar[i].kullanimSekli, sizeof(yeniNode->receteBilgisi.ilaclar[i].kullanimSekli)-1);
        yeniNode->receteBilgisi.ilaclar[i].kullanimSekli[sizeof(yeniNode->receteBilgisi.ilaclar[i].kullanimSekli)-1] = '\0';
        yeniNode->receteBilgisi.ilaclar[i].adet = ilaclar[i].adet;
    }
    yeniNode->next = stack->top;
    stack->top = yeniNode;
    stack->size++;
    printf("Recete olusturuldu. Recete No: %s\n", yeniNode->receteBilgisi.receteNo);
}

// receteYazdir: Reçete stack'indeki tüm reçeteleri ekrana yazdırır
void receteYazdir(ReceteStack* stack) {
    if(stack->size == 0) {
        printf("Recete listesi bos!\n");
        return;
    }
    ReceteNode* current = stack->top;
    printf("\n--- Receteler ---\n");
    while(current != NULL) {
        printf("Recete No: %s\nHasta TC Kimlik No: %s\nDoktor ID: %s\nTarih: %s\n",
               current->receteBilgisi.receteNo,
               current->receteBilgisi.hastaId,
               current->receteBilgisi.doktorId,
               current->receteBilgisi.tarih);
        printf("Ilaclar:\n");
        int i=0;
        for(i = 0; i < current->receteBilgisi.ilacSayisi; i++) {
            printf("  Ilac Adi: %s, Doz: %s, Kullanim Sekli: %s, Adet: %d\n",
                   current->receteBilgisi.ilaclar[i].ilacAdi,
                   current->receteBilgisi.ilaclar[i].doz,
                   current->receteBilgisi.ilaclar[i].kullanimSekli,
                   current->receteBilgisi.ilaclar[i].adet);
        }
        printf("----------------------\n");
        current = current->next;
    }
}

// hastaToHL7: Hasta bilgilerini HL7 mesaj formatına dönüştürür (hasta tanımlayıcı olarak TC Kimlik No kullanılır)
HL7Message hastaToHL7(Hasta hasta, TriyajBilgisi triyaj) {
    HL7Message mesaj;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char currentTime[20];
    snprintf(currentTime, sizeof(currentTime), "%04d%02d%02d%02d%02d%02d",
             tm_info->tm_year+1900, tm_info->tm_mon+1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    snprintf(mesaj.hl7Format.messageHeader, sizeof(mesaj.hl7Format.messageHeader),
             "MSH|^~\\&|ACILSERVIS|HASTANE|LAB|HASTANE|%s||ADT^A01|%s|P|2.5",
             currentTime, receteNoUret());
    snprintf(mesaj.hl7Format.patientInfo, sizeof(mesaj.hl7Format.patientInfo),
             "PID|||%s||%s^%s||%s|%c|||%s||%s",
             hasta.tcKimlikNo, hasta.ad, hasta.soyad, hasta.dogumTarihi,
             hasta.cinsiyet, hasta.adres, hasta.telefon);
    snprintf(mesaj.hl7Format.observationInfo, sizeof(mesaj.hl7Format.observationInfo),
             "OBX|1|NM|OncelikPuani||%d", hasta.OncelikPuani);
    strncpy(mesaj.hl7Format.medicationInfo, "", sizeof(mesaj.hl7Format.medicationInfo));
    snprintf(mesaj.hl7Format.messageFooter, sizeof(mesaj.hl7Format.messageFooter), "FTS|1");
    snprintf(mesaj.rawHL7, sizeof(mesaj.rawHL7), "%s\n%s\n%s\n%s\n%s",
             mesaj.hl7Format.messageHeader,
             mesaj.hl7Format.patientInfo,
             mesaj.hl7Format.observationInfo,
             mesaj.hl7Format.medicationInfo,
             mesaj.hl7Format.messageFooter);
    return mesaj;
}

// hl7MesajGonder: HL7 mesajını ekrana basarak gönderim simülasyonu yapar
void hl7MesajGonder(HL7Message mesaj) {
    printf("\n--- HL7 Mesaji Gonderiliyor ---\n%s\n", mesaj.rawHL7);
}

// hl7ToHasta: HL7 mesajından hasta bilgisini okur (hastaNumarasi boş bırakılır)
Hasta hl7ToHasta(HL7Message mesaj) {
    Hasta hasta;
    sscanf(mesaj.hl7Format.patientInfo, "PID|||%19[^|]||%49[^|]^%49[^|]||%10[^|]|%c|||%99[^|]||%14[^|]",
           hasta.tcKimlikNo, hasta.ad, hasta.soyad, hasta.dogumTarihi, &hasta.cinsiyet, hasta.adres, hasta.telefon);
    hasta.OncelikPuani = 0;
    strcpy(hasta.hastaNumarasi, "");
    return hasta;
}

// tumHastalariListele: Hasta kuyruğundaki tüm hastaları listeler
void tumHastalariListele(HastaQueue* queue) {
    if(queue->size == 0) {
        printf("Sirada bos.\n");
        return;
    }
    printf("\n--- Hasta Listesi ---\n");
    HastaNode* temp = queue->front;
    while(temp != NULL) {
        printf("Hasta Numarasi: %s, Ad: %s, Soyad: %s, Oncelik Puani: %d, Renk: %s\n",
               temp->hastaBilgisi.hastaNumarasi,
               temp->hastaBilgisi.ad,
               temp->hastaBilgisi.soyad,
               temp->hastaBilgisi.OncelikPuani,
               temp->triyajBilgisi.color);
        temp = temp->next;
    }
}

// menuGoster: Ana menüyü gösterir ve kullanıcı girişlerine göre ilgili fonksiyonları çağırır
void menuGoster(HastaQueue* hastaQueue, ReceteStack* receteStack) {
    int secim;
    do {
        printf("\n--- Acil Servis Uygulamasi Menu ---\n");
        printf("1. Hasta Kaydi Ekle\n");
        printf("2. Hasta Bilgilerini Listele\n");
        printf("3. Muayene Yap\n");
        printf("4. Receteleri Yazdir\n");
        printf("5. HL7 Mesaji Gonder\n");
        printf("6. Cikis\n");
        printf("Seciminiz: ");
        scanf("%d", &secim);
        getchar(); // Newline karakterini temizler

        if(secim == 1) {
            Hasta yeniHasta;
            TriyajBilgisi triyaj;
            // TC Kimlik No manuel girilir
            printf("\nTC Kimlik No: ");
            fgets(yeniHasta.tcKimlikNo, sizeof(yeniHasta.tcKimlikNo), stdin);
            yeniHasta.tcKimlikNo[strcspn(yeniHasta.tcKimlikNo, "\n")] = '\0';
            // Otomatik hasta numarası oluşturulur
            hastaNumarasiCounter++;
            sprintf(yeniHasta.hastaNumarasi, "%d", hastaNumarasiCounter);
            printf("Ad: ");
            fgets(yeniHasta.ad, sizeof(yeniHasta.ad), stdin);
            yeniHasta.ad[strcspn(yeniHasta.ad, "\n")] = '\0';
            printf("Soyad: ");
            fgets(yeniHasta.soyad, sizeof(yeniHasta.soyad), stdin);
            yeniHasta.soyad[strcspn(yeniHasta.soyad, "\n")] = '\0';
            printf("Dogum Tarihi (YYYY-MM-DD): ");
            fgets(yeniHasta.dogumTarihi, sizeof(yeniHasta.dogumTarihi), stdin);
            yeniHasta.dogumTarihi[strcspn(yeniHasta.dogumTarihi, "\n")] = '\0';
            printf("Cinsiyet (E/K): ");
            scanf(" %c", &yeniHasta.cinsiyet);
            getchar();
            printf("Adres: ");
            fgets(yeniHasta.adres, sizeof(yeniHasta.adres), stdin);
            yeniHasta.adres[strcspn(yeniHasta.adres, "\n")] = '\0';
            printf("Telefon: ");
            fgets(yeniHasta.telefon, sizeof(yeniHasta.telefon), stdin);
            yeniHasta.telefon[strcspn(yeniHasta.telefon, "\n")] = '\0';
            // Triyaj bilgileri alınır
            printf("Sikayet: ");
            fgets(triyaj.sikayet, sizeof(triyaj.sikayet), stdin);
            triyaj.sikayet[strcspn(triyaj.sikayet, "\n")] = '\0';
            printf("Hayati Tehlike (E/H): ");
            scanf(" %c", &triyaj.hayatiTehlikesi);
            getchar();
            printf("Ates Degeri: ");
            scanf("%f", &triyaj.atesDegeri);
            getchar();
            printf("Gelis Tarihi (YYYY-MM-DD): ");
            fgets(triyaj.gelisTarihi, sizeof(triyaj.gelisTarihi), stdin);
            triyaj.gelisTarihi[strcspn(triyaj.gelisTarihi, "\n")] = '\0';
            printf("Gelis Saati (HH:MM:SS): ");
            fgets(triyaj.gelisSaati, sizeof(triyaj.gelisSaati), stdin);
            triyaj.gelisSaati[strcspn(triyaj.gelisSaati, "\n")] = '\0';
            hastaKabul(hastaQueue, yeniHasta, triyaj);
        }
        else if(secim == 2) {
            tumHastalariListele(hastaQueue);
        }
        else if(secim == 3) {
            if(hastaQueue->size == 0) {
                printf("Muayene yapmak icin hasta yok!\n");
            } else {
                Hasta *hasta = hastaGetir(hastaQueue);
                printf("Hasta muayenesi yapildi.\n");
                char cevap;
                printf("Recete olusturulsun mu? (E/H): ");
                scanf(" %c", &cevap);
                getchar();
                if(cevap == 'E' || cevap == 'e') {
                    char doktorId[20];
                    printf("Doktor ID: ");
                    fgets(doktorId, sizeof(doktorId), stdin);
                    doktorId[strcspn(doktorId, "\n")] = '\0';
                    int ilacSayisi;
                    printf("Ilac sayisini girin: ");
                    scanf("%d", &ilacSayisi);
                    getchar();
                    Ilac* ilaclar = (Ilac*)malloc(sizeof(Ilac) * ilacSayisi);
                    if(ilaclar == NULL) {
                        fprintf(stderr, "Ilaclar icin bellek tahsisi basarisiz!\n");
                        exit(EXIT_FAILURE);
                    }
                    int i=0;
                    for(i = 0; i < ilacSayisi; i++) {
                        printf("Ilac %d icin bilgileri:\n", i+1);
                        printf("   Ilac Adi: ");
                        fgets(ilaclar[i].ilacAdi, sizeof(ilaclar[i].ilacAdi), stdin);
                        ilaclar[i].ilacAdi[strcspn(ilaclar[i].ilacAdi, "\n")] = '\0';
                        printf("   Doz: ");
                        fgets(ilaclar[i].doz, sizeof(ilaclar[i].doz), stdin);
                        ilaclar[i].doz[strcspn(ilaclar[i].doz, "\n")] = '\0';
                        printf("   Kullanim Sekli: ");
                        fgets(ilaclar[i].kullanimSekli, sizeof(ilaclar[i].kullanimSekli), stdin);
                        ilaclar[i].kullanimSekli[strcspn(ilaclar[i].kullanimSekli, "\n")] = '\0';
                        printf("   Adet: ");
                        scanf("%d", &ilaclar[i].adet);
                        getchar();
                    }
                    receteOlustur(receteStack, *hasta, doktorId, ilaclar, ilacSayisi);
                    free(ilaclar);
                }
                hastaSil(hastaQueue);
            }
        }
        else if(secim == 4) {
            receteYazdir(receteStack);
        }
        else if(secim == 5) {
            if(hastaQueue->size == 0) {
                printf("HL7 mesaji gondermek icin hasta yok.\n");
            } else {
                Hasta *hasta = hastaGetir(hastaQueue);
                HL7Message mesaj = hastaToHL7(*hasta, hastaQueue->front->triyajBilgisi);
                hl7MesajGonder(mesaj);
            }
        }
        else if(secim == 6) {
            printf("Cikis yapiliyor...\n");
        }
        else {
            printf("Gecersiz secim, tekrar deneyin.\n");
        }
    } while(secim != 6);
}

// freeHastaQueue: Hasta kuyruğunda ayrılan belleği serbest bırakır
void freeHastaQueue(HastaQueue* queue) {
    while(queue->front != NULL) {
        HastaNode* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }
}

// freeReceteStack: Reçete stack'inde ayrılan belleği serbest bırakır
void freeReceteStack(ReceteStack* stack) {
    while(stack->top != NULL) {
        ReceteNode* temp = stack->top;
        stack->top = stack->top->next;
        if(temp->receteBilgisi.ilaclar != NULL)
            free(temp->receteBilgisi.ilaclar);
        free(temp);
    }
}

// main: Programın başlangıç noktası, veri yapıları oluşturulur, ana menü çalıştırılır ve bellek temizlenir.
int main() {
    srand(time(NULL));  // Rastgele sayı üretilmesi için tohumlama

    HastaQueue hastaQueue;
    ReceteStack receteStack;
    hastaQueueInit(&hastaQueue);
    receteStackInit(&receteStack);

    menuGoster(&hastaQueue, &receteStack);

    freeHastaQueue(&hastaQueue);
    freeReceteStack(&receteStack);

    return 0;
}