# Hastane Acil Servis Yönetim Sistemi

## Proje Tanımı
Bu uygulama, hastane acil servislerinde kullanılmak üzere tasarlanmış, hastaların kabul edilmesi, triyaj değerlendirmesi, muayene süreci ve reçete düzenlenmesi süreçlerini yöneten kapsamlı bir C programıdır. Sistem, acil servislerde karşılaşılan hasta önceliklendirme probleminin verimli bir şekilde çözülmesini hedeflemektedir.

## Özellikler

### Temel İşlevler
- ✅ Hasta kayıt işlemleri (TC Kimlik No, kişisel bilgiler)
- ✅ Otomatik hasta numarası atama
- ✅ Şikayete dayalı triyaj değerlendirmesi
- ✅ Aciliyet seviyesi ve renk kodu belirleme (Kırmızı, Turuncu, Sarı, Yeşil, Mavi)
- ✅ Hastaları öncelik puanına göre otomatik sıralama
- ✅ Muayene işlemleri
- ✅ Reçete oluşturma ve yönetme
- ✅ HL7 mesaj formatı desteği

### Veri Yapıları
- **Öncelikli Kuyruk (Priority Queue):** Hastaların aciliyet durumlarına göre sıralanması
- **Yığın (Stack):** Reçetelerin yönetimi
- **Bağlı Listeler (Linked Lists):** Dinamik veri saklama ve yönetimi

## Triyaj Sistemi

Uygulama, aşağıdaki 5 seviyeli triyaj sistemini kullanmaktadır:

| Aciliyet Seviyesi | Renk   | Öncelik Puanı | Örnek Durumlar                                  |
|-------------------|--------|---------------|------------------------------------------------|
| 1 (En acil)       | Kırmızı| 100           | Hayati tehlike, şok belirtileri, kalp durması   |
| 2                 | Turuncu| 80            | Şiddetli ağrı, yoğun kanama, göğüs ağrısı       |
| 3                 | Sarı   | 60            | Orta şiddette ağrı, kırık şüphesi              |
| 4                 | Yeşil  | 40            | Hafif burkulmalar, soğuk algınlığı, hafif ateş  |
| 5 (En az acil)    | Mavi   | 20            | Acil olmayan durumlar                          |

* **Not:** 39°C üzeri ateş durumunda aciliyet seviyesi otomatik olarak yükseltilir.

## Sistem Akış Diyagramı

![Blank diagram](https://github.com/user-attachments/assets/a7b052f7-abfc-4e04-ab27-1bccfa34d9f6)

## Nasıl Kullanılır

### Derleme ve Çalıştırma
```bash
# Projeyi derleyin
gcc -o acil_servis main.c

# Uygulamayı çalıştırın
./acil_servis
```

### Menü Seçenekleri
Program çalıştırıldığında aşağıdaki menü karşınıza çıkacaktır:
```
--- Acil Servis Uygulamasi Menu ---
1. Hasta Kaydi Ekle
2. Hasta Bilgilerini Listele
3. Muayene Yap
4. Receteleri Yazdir
5. HL7 Mesaji Gonder
6. Cikis
```

#### 1. Hasta Kaydı Ekle
Yeni hasta bilgilerini ve şikayetlerini sisteme girmenizi sağlar. Triyaj otomatik olarak hesaplanır.

#### 2. Hasta Bilgilerini Listele
Sistemdeki tüm hastaları öncelik sırasına göre listeler.

#### 3. Muayene Yap
En yüksek önceliğe sahip hastayı muayene eder ve isterseniz reçete oluşturmanıza olanak tanır.

#### 4. Reçeteleri Yazdır
Sistemdeki tüm reçeteleri görüntüler.

#### 5. HL7 Mesajı Gönder
En öndeki hasta için HL7 formatında mesaj oluşturur ve simüle edilmiş bir gönderim işlemi gerçekleştirir.

## Teknik Detaylar

### Proje Yapısı
Uygulama, aşağıdaki ana bileşenlerden oluşmaktadır:

1. **Hasta Yönetimi**: TC Kimlik No, kişisel bilgiler ve öncelik puanı
2. **Triyaj Sistemi**: Şikayet ve durum değerlendirmesi
3. **Reçete Modülü**: İlaç bilgileri ve reçete kayıtları
4. **HL7 Entegrasyonu**: Sağlık sistemleri arası iletişim

### Veri Tipleri
- `Hasta`: Hasta bilgilerini tutan yapı
- `TriyajBilgisi`: Aciliyet değerlendirme bilgileri
- `Ilac`: İlaç bilgileri
- `Recete`: Reçete detayları
- `HL7Message`: HL7 formatında mesaj yapısı

## Katkıda Bulunma
Projeye katkıda bulunmak için lütfen bir "fork" oluşturun ve "pull request" gönderin. Büyük değişiklikler için önce bir "issue" açarak tartışmanızı öneririz.

## Gelecek Geliştirmeler
- [ ] Veritabanı entegrasyonu
- [ ] Çoklu kullanıcı desteği (doktor/hemşire rolleri)
- [ ] İstatistik raporları
- [ ] Hasta takip sistemi
- [ ] Gelişmiş HL7 entegrasyonu

## Lisans
Bu proje [MIT Lisansı](LICENSE) altında lisanslanmıştır.

## İletişim
Sorularınız veya önerileriniz için [e-posta adresiniz veya GitHub kullanıcı adınız] üzerinden iletişime geçebilirsiniz.
