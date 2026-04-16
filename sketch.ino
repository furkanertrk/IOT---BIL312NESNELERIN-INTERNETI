#include <Wire.h> // I2C iletişimi için gerekli kütüphane
#include <LiquidCrystal_I2C.h> // I2C LCD ekranı kontrol etmek için kütüphane
#include "DHT.h" // DHT sensörü (sıcaklık ve nem) kütüphanesi

#define DHTPIN 15 // DHT22 veri pininin ESP32'de bağlı olduğu GPIO pini
#define DHTTYPE DHT22 // Kullanılan DHT sensörünün modeli (DHT22)
DHT dht(DHTPIN, DHTTYPE); // DHT nesnesini tanımlıyoruz

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 adresli, 16 sütun ve 2 satırlı LCD nesnesi

const int ldrPin = 34; // LDR sensörünün analog okuma pini (AO)
const int buttonPin = 18; // LCD süresini değiştirecek butonun pini (pull-up)
const int redPin = 25; // RGB LED'in Kırmızı pini
const int greenPin = 26; // RGB LED'in Yeşil pini
const int bluePin = 27; // RGB LED'in Mavi pini

// Zamanlama (Round Robin ve millis) değişkenleri
unsigned long previousMillisLCD = 0; // LCD ekranın son güncellenme zamanı
unsigned long previousMillisSerial = 0; // Seri portun son yazdırılma zamanı
unsigned long previousMillisButton = 0; // Buton okuma (debounce) için son zaman
int displayState = 0; // LCD'de gösterilecek ekranın sırası (0: Nem, 1: Sıcaklık, 2: Işık)
int lcdInterval = 2000; // LCD ekranlar arası geçiş süresi (başlangıçta 2000 ms yani 2 sn)
int lastButtonState = HIGH; // Butonun bir önceki durumu (pull-up olduğu için varsayılan HIGH)

// LDR lux hesaplaması için sabitler (Wokwi standart LDR dönüşüm formülü)
const float GAMMA = 0.7; // LDR'nin gamma değeri
const float RL10 = 50; // 10 lux altındaki direnç değeri

void setup() {
  Serial.begin(115200); // Seri haberleşmeyi başlatıyoruz
  dht.begin(); // DHT sensörünü başlatıyoruz
  lcd.init(); // LCD ekranı başlatıyoruz
  lcd.backlight(); // LCD arka ışığını açıyoruz

  pinMode(ldrPin, INPUT); // LDR pinini giriş olarak ayarlıyoruz
  pinMode(buttonPin, INPUT_PULLUP); // Buton pinini dahili pull-up direnci ile giriş ayarlıyoruz
  pinMode(redPin, OUTPUT); // Kırmızı LED pinini çıkış ayarlıyoruz
  pinMode(greenPin, OUTPUT); // Yeşil LED pinini çıkış ayarlıyoruz
  pinMode(bluePin, OUTPUT); // Mavi LED pinini çıkış ayarlıyoruz
}

void loop() {
  unsigned long currentMillis = millis(); // Sistem çalışmaya başladığından beri geçen süreyi (ms) alıyoruz

  // 1. BUTON OKUMA VE DEBOUNCE MANTIĞI
  int reading = digitalRead(buttonPin); // Butonun anlık durumunu okuyoruz
  // Eğer butona basıldıysa (LOW) ve dalgalanma (debounce) süresi geçtiyse
  if (reading == LOW && lastButtonState == HIGH && (currentMillis - previousMillisButton > 200)) {
    lcdInterval = (lcdInterval == 2000) ? 4000 : 2000; // Süreyi 2000 ise 4000, 4000 ise 2000 yap
    Serial.print("LCD suresi degistirildi: "); // Seri ekrana mesajın ilk kısmını yazdır
    Serial.print(lcdInterval / 1000); // Saniye cinsinden süreyi yazdır
    Serial.println(" sn"); // Mesajı bitir ve alt satıra geç
    previousMillisButton = currentMillis; // Buton basılma zamanını güncelle
  }
  lastButtonState = reading; // Bir sonraki döngü için buton durumunu kaydet

  // 2. SENSÖR OKUMALARI
  float h = dht.readHumidity(); // DHT22'den nem değerini okuyoruz
  float t = dht.readTemperature(); // DHT22'den sıcaklık değerini okuyoruz
  int analogValue = analogRead(ldrPin); // LDR'den analog (ham) ışık değerini okuyoruz

  // LDR ADC değerinden Lux değerine dönüşüm
  float voltage = analogValue / 4095.0 * 3.3; // 12-bit ADC değerini voltaja çeviriyoruz
  float resistance = 10000.0 * voltage / (3.3 - voltage); // 10kOhm voltaj bölücü tam formülü
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA)); // Direnci Lux değerine çeviriyoruz

  // 3. SERİ MONİTÖR (Her 3 saniyede bir yazdırma)
  if (currentMillis - previousMillisSerial >= 3000) { // Son yazdırmadan bu yana 3 saniye geçtiyse
    previousMillisSerial = currentMillis; // Zamanlayıcıyı güncelle
    
    // Format: Nem: 49.00% || Sıcaklık: -40.00°C || Işık: 50 lux
    Serial.print("Nem: "); 
    Serial.print(h); 
    Serial.print("% || Sicaklik: "); 
    Serial.print(t); 
    Serial.print("°C || Isik: "); 
    Serial.print((int)lux); 
    Serial.println(" lux"); 
  }

  // 4. RGB LED KONTROLÜ
  if (lux < 10) { // Ortam karanlık ise (< 10 lux)
    setRGB(0, 0, 255); // Mavi
  } else { // Ortam aydınlık ise
    if (t < 0) { // Sıcaklık LOW
      setRGB(0, 150, 255); // Açık mavi
    } else if (t >= 0 && t <= 30) { // Sıcaklık NORMAL
      setRGB(0, 255, 0); // Yeşil
    } else { // Sıcaklık HIGH
      setRGB(255, 165, 0); // Turuncu
    }
  }

  // 5. I2C LCD EKRAN KONTROLÜ (Round Robin Algoritması)
  if (currentMillis - previousMillisLCD >= lcdInterval) { // Beklenen gösterim süresi dolduysa
    previousMillisLCD = currentMillis; // LCD zamanlayıcısını güncelle
    lcd.clear(); // Ekranı temizle

    if (displayState == 0) { // Sıra Nem ekranındaysa
      lcd.setCursor(0, 0); lcd.print("NEM"); // NEM yazdır
      lcd.setCursor(15, 0); lcd.print(lcdInterval / 1000); // Süreyi en sağ üste yazdır
      
      lcd.setCursor(0, 1); lcd.print((int)h); // Tam sayı olarak nemi yazdır
      lcd.setCursor(12, 1); // İmleci sağ alta doğru al
      if (h <= 60) lcd.print("LOW"); // %60 ve altı
      else lcd.print("HIGH"); // %61 ve üstü
      
      displayState = 1; // Bir sonraki duruma geç
    }
    else if (displayState == 1) { // Sıra Sıcaklık ekranındaysa
      lcd.setCursor(0, 0); lcd.print("SICAKLIK"); // SICAKLIK yazdır
      lcd.setCursor(15, 0); lcd.print(lcdInterval / 1000); // Süreyi en sağ üste yazdır
      
      lcd.setCursor(0, 1); lcd.print((int)t); // Tam sayı olarak sıcaklığı yazdır
      lcd.setCursor(8, 1); 
      if (t < 0) lcd.print("VERY LOW"); 
      else if (t <= 30) lcd.print("NORMAL"); 
      else lcd.print("HIGH"); 
      
      displayState = 2; // Bir sonraki duruma geç
    }
    else if (displayState == 2) { // Sıra Işık ekranındaysa
      lcd.setCursor(0, 0); lcd.print("ISIK"); // ISIK yazdır
      lcd.setCursor(15, 0); lcd.print(lcdInterval / 1000); // Süreyi en sağ üste yazdır
      
      lcd.setCursor(0, 1); 
      if (lux < 10) lcd.print("KARANLIK"); 
      else lcd.print("AYDINLIK"); 
      
      displayState = 0; // Başa (Neme) dön
    }
  }
}

// RGB LED'e renk değerlerini göndermek için yardımcı fonksiyon
void setRGB(int r, int g, int b) {
  analogWrite(redPin, r); // Kırmızı pine PWM sinyali gönder
  analogWrite(greenPin, g); // Yeşil pine PWM sinyali gönder
  analogWrite(bluePin, b); // Mavi pine PWM sinyali gönder
}