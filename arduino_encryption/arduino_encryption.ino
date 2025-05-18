#include <AESLib.h>

AESLib aesLib;

// 128-бітний ключ шифрування (16 байт)
byte aes_key[] = { 57, 36, 24, 25, 28, 86, 32, 41, 31, 36, 91, 36, 51, 74, 63, 89 };

// Вектор ініціалізації (16 байт)
byte aes_iv[16] = { 0x79, 0x4E, 0x98, 0x21, 0xAE, 0xD8, 0xA6, 0xAA, 0xD7, 0x97, 0x44, 0x14, 0xAB, 0xDD, 0x9F, 0x2C };

// Буфери для тексту
#define BUFFER_SIZE 128
char plaintext[BUFFER_SIZE] = "Цей текст буде зашифрований за допомогою AES-128";
byte ciphertext[BUFFER_SIZE] = {0};
char decryptedtext[BUFFER_SIZE] = {0};

void setup() {
  Serial.begin(9600);
  while (!Serial); // Чекаємо підключення Serial
  
  // Ініціалізація AES
  aesLib.set_paddingmode(paddingMode::CMS);
  
  Serial.println("Початок роботи AES-128 шифрування тексту");
  Serial.println("----------------------------------------");
  Serial.print("Оригінальний текст: ");
  Serial.println(plaintext);
  Serial.print("Довжина тексту: ");
  Serial.println(strlen(plaintext));
}

void loop() {
  // Копіюємо IV для кожного циклу
  byte enc_iv[16];
  memcpy(enc_iv, aes_iv, sizeof(aes_iv));
  
  // Шифрування
  uint16_t cipherLength = aesLib.encrypt((byte*)plaintext, strlen(plaintext), ciphertext, aes_key, sizeof(aes_key), enc_iv);
  
  Serial.print("Зашифрований текст (HEX): ");
  printHex(ciphertext, cipherLength);
  Serial.println();
  
  // Копіюємо оригінальний IV для дешифрування
  memcpy(enc_iv, aes_iv, sizeof(aes_iv));
  
  // Дешифрування
  uint16_t decryptedLength = aesLib.decrypt(ciphertext, cipherLength, (byte*)decryptedtext, aes_key, sizeof(aes_key), enc_iv);
  decryptedtext[decryptedLength] = '\0'; // Додаємо термінатор рядка
  
  Serial.print("Розшифрований текст: ");
  Serial.println(decryptedtext);
  
  // Перевірка коректності
  if (strcmp(plaintext, decryptedtext) == 0) {
    Serial.println("Перевірка: шифрування/дешифрування пройшло успішно!");
  } else {
    Serial.println("Помилка: розшифрований текст не співпадає з оригіналом!");
    Serial.print("Очікувано: ");
    Serial.println(plaintext);
    Serial.print("Отримано: ");
    Serial.println(decryptedtext);
    Serial.print("Довжина оригіналу: ");
    Serial.println(strlen(plaintext));
    Serial.print("Довжина розшифрованого: ");
    Serial.println(strlen(decryptedtext));
  }
  
  Serial.println("----------------------------------------");
  delay(5000); // Затримка 5 секунд між ітераціями
}

// Допоміжна функція для виводу HEX
void printHex(byte *data, uint16_t length) {
  for (int i = 0; i < length; i++) {
    if (data[i] < 0x10) Serial.print('0');
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
}