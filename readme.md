# ESP32 Tabanlı Ortam Verisi İzleme Sistemi (Sıcaklık, Nem ve Işık)

**Hazırlayan:** Furkan Ertürk

**Ders:** Nesnelerin İnterneti (IoT) – 1. Ödev

---

## Proje Tanımı

Bu çalışmada, ESP32 mikrodenetleyicisi temel alınarak çevresel sıcaklık, bağıl nem ve ortam aydınlık seviyesini eş zamanlı olarak ölçen ve raporlayan bir gömülü sistem tasarlanmıştır. Sıcaklık ve nem ölçümleri için DHT22 sensörü, ışık şiddeti ölçümü için ise bir LDR (Light Dependent Resistor) tercih edilmiştir.

Ölçüm verileri, 16x2 boyutlarında bir I2C LCD ekran üzerinde kullanıcıya sunulmaktadır. Ekranın sınırlı satır kapasitesi nedeniyle üç parametrenin aynı anda gösterimi mümkün olmadığından, veriler Round Robin (sıralı döngü) yöntemiyle dönüşümlü olarak görüntülenmektedir. Sisteme ek olarak bir RGB LED entegre edilmiş; böylece kullanıcının ekrana bakmaksızın, yalnızca renk değişimini gözlemleyerek ortamın genel durumu hakkında hızlı bir çıkarım yapabilmesi amaçlanmıştır.

## Kullanılan Kütüphaneler

- **DHT sensor library** *(Adafruit)*: DHT22 sensörü ile mikrodenetleyici arasındaki haberleşmeyi yönetir. Stabil ve güvenilir bir kütüphane olduğu test sürecinde doğrulanmıştır.
- **LiquidCrystal I2C**: LCD ekranın I2C protokolü üzerinden sürülmesini sağlar. Bu yaklaşım, paralel bağlantıya kıyasla pin kullanımını minimize ederek devre karmaşıklığını belirgin biçimde azaltmaktadır.

## Simülasyon Bağlantısı

🔗 https://wokwi.com/projects/461358818030312449

## Çalıştırma Prosedürü

1. Yukarıda belirtilen Wokwi bağlantısı üzerinden proje sayfasına erişiniz.
2. Sol panelde yer alan yeşil **"Play"** butonuna tıklayarak simülasyonu başlatınız.
3. Simülasyon süresince **Seri Monitör**'ün açık tutulması önerilir; sensör verileri üç saniyelik periyotlarla bu arayüze de aktarılmaktadır.
4. LCD ekran üzerinde nem, sıcaklık ve ışık değerlerinin ikişer saniyelik aralıklarla dönüşümlü olarak görüntülendiği gözlemlenecektir. Bu davranış, daha önce belirtilen Round Robin mantığının doğrudan bir uygulamasıdır.
5. Devre üzerinde konumlandırılmış olan yeşil **push-button**'a basıldığında, LCD'deki geçiş süresi iki saniyeden dört saniyeye çıkmaktadır. Söz konusu değişim Seri Monitör çıktıları üzerinden de doğrulanabilir.
6. Farklı senaryoların test edilmesi amacıyla **DHT22** ve **LDR** sensörlerine tıklanarak açılan slider'lar aracılığıyla sıcaklık, nem ve ışık değerleri istenen seviyelere ayarlanabilir.
7. **RGB LED**, anlık ölçüm değerlerine bağlı olarak aşağıdaki renk kodlamasıyla geri bildirim sağlamaktadır:

   - 🔵 **Mavi (0, 0, 255)** → Ortam karanlık (< 10 lux). Sıcaklık değeri dikkate alınmaz.
   - 🩵 **Açık Mavi (0, 150, 255)** → Aydınlık ortam + donma seviyesinin altında sıcaklık (T < 0°C, **VERY LOW**)
   - 🟢 **Yeşil (0, 255, 0)** → Aydınlık ortam + normal sıcaklık aralığı (0°C ≤ T ≤ 30°C, **NORMAL**)
   - 🟠 **Turuncu (255, 165, 0)** → Aydınlık ortam + yüksek sıcaklık (T > 30°C, **HIGH**)

LED'in koşullara verdiği hızlı tepki, sistemin sensör verilerini doğru biçimde okuduğunu ve tanımlanan eşik değerlerini tutarlı biçimde yorumladığını ortaya koymaktadır.
