#define BLYNK_TEMPLATE_ID "TMPL6-KdNixZ0"
#define BLYNK_TEMPLATE_NAME "led matrix"
#define BLYNK_AUTH_TOKEN "UjIXo2HJUkWNaZGFiDFqGcdzaVzlrSZA"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <string.h>
#include "audio_data.h"

#define MAX_LENGTH 20
#define MAX_QUEUE_SIZE 20

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

int buttonPin = 18;
bool buttonPressed = false;
bool turnOn = false;

int gndPins[4] = {4, 0, 2, 15};
int vccPins[8] = {14, 27, 26, 25, 33, 32, 12, 13};

// UTF-8-safe queue using StringsS
String queue[MAX_QUEUE_SIZE];
int queueSize = 0;

// ---------------------------
// QUEUE FUNCTIONS
// ---------------------------
bool enqueue(String word) {
    if (queueSize >= MAX_QUEUE_SIZE) {
        Serial.println("Queue full!");
        return false;
    }
    queue[queueSize++] = word;
    return true;
}

bool dequeue() {
    if (queueSize <= 0) {
        Serial.println("Queue empty!");
        return false;
    }
    for (int i = 1; i < queueSize; i++) {
        queue[i - 1] = queue[i];
    }
    queueSize--;
    return true;
}

// ---------------------------
// LETTER → PIN MAPPING
// ---------------------------
struct LetterMap {
    String letter;
    int gndIndex;
    int vccIndex;
};

LetterMap alphabet[] = {
    {"А", 0, 0}, {"Б", 0, 1}, {"В", 0, 2}, {"Г", 0, 3},
    {"Д", 0, 4}, {"Е", 0, 5}, {"Ж", 0, 6}, {"З", 0, 7},
    {"И", 1, 0}, {"Й", 1, 1}, {"К", 1, 2}, {"Л", 1, 3},
    {"М", 1, 4}, {"Н", 1, 5}, {"О", 1, 6}, {"П", 1, 7},
    {"Р", 2, 0}, {"С", 2, 1}, {"Т", 2, 2}, {"У", 2, 3},
    {"Ф", 2, 4}, {"Х", 2, 5}, {"Ц", 2, 6}, {"Ч", 2, 7},
    {"Ш", 3, 0}, {"Щ", 3, 1}, {"Ъ", 3, 2}, {"Ы", 3, 3},
    {"Ь", 3, 4}, {"Э", 3, 5}, {"Ю", 3, 6}, {"Я", 3, 7}
};

int alphabetSize = sizeof(alphabet) / sizeof(alphabet[0]);

bool getLetterPins(String c, int &gndPin, int &vccPin) {
    for (int i = 0; i < alphabetSize; i++) {
        if (alphabet[i].letter == c) {
            gndPin = gndPins[alphabet[i].gndIndex];
            vccPin = vccPins[alphabet[i].vccIndex];
            return true;
        }
    }
    return false;
}

void lightLetter(String c, int duration) {
    int gndPin, vccPin;

    if (!getLetterPins(c, gndPin, vccPin)) {
        Serial.print("Unknown letter: ");
        Serial.println(c);
        // Turn OFF all pins
        for (int g = 0; g < 4; g++) digitalWrite(gndPins[g], HIGH);
        for (int v = 0; v < 8; v++) digitalWrite(vccPins[v], LOW);
        return;
    }

    // Turn OFF all pins before lighting next letter
    for (int g = 0; g < 4; g++) digitalWrite(gndPins[g], HIGH);
    for (int v = 0; v < 8; v++) digitalWrite(vccPins[v], LOW);

    // Light the LED
    digitalWrite(gndPin, LOW);
    digitalWrite(vccPin, HIGH);

    delay(duration);

    // Turn off
    digitalWrite(gndPin, HIGH);
    digitalWrite(vccPin, HIGH);
}

// ---------------------------
// SPLIT INPUT STRING INTO WORDS
// ---------------------------
void splitToArray(String data) {
    int start = 0;
    int commaIndex;

    while ((commaIndex = data.indexOf(',', start)) != -1) {
        String w = data.substring(start, commaIndex);
        w.trim();
        w.toUpperCase();
        enqueue(w);
        start = commaIndex + 1;
    }

    String w = data.substring(start);
    w.trim();
    w.toUpperCase();
    enqueue(w);
}

// ---------------------------
// BLYNK CALLBACK
// ---------------------------
BLYNK_WRITE(V0) {
    String incoming = param.asString();
    Serial.println("Received: " + incoming);

    splitToArray(incoming);

    Serial.println("Parsed Words:");
    for (int i = 0; i < queueSize; i++) {
        Serial.println(queue[i]);
    }

    turnOn = true;

    // Initialize pins
    pinMode(buttonPin, INPUT_PULLUP);
    for (int g = 0; g < 4; g++) {
        pinMode(gndPins[g], OUTPUT);
        digitalWrite(gndPins[g], HIGH);
    }
    for (int v = 0; v < 8; v++) {
        pinMode(vccPins[v], OUTPUT);
        digitalWrite(vccPins[v], LOW);
    }
}

// ...existing code...
unsigned char* gen_spi2() {
    printf("Simulating microphone with %u bytes of data\n", output_raw_len);
    
    // Simulate reading 512 bytes at a time
    int buffer_size = 512;
    for (int offset = 0; offset < output_raw_len; offset += buffer_size) {
        
        // Calculate how many bytes to "read" (don't go past the end)
        int bytes_to_read = (output_raw_len - offset < buffer_size) ? 
                             (output_raw_len - offset) : buffer_size;

        // Pointer to the current position in the simulated data
        unsigned char* current_buffer = &output_raw[offset];
        return current_buffer;
        // --- Pass 'current_buffer' to your Speech-to-Text logic here ---
        // process_audio(current_buffer, bytes_to_read);
    }
} 
// ...existing code...

// ---------------------------
// SETUP
// ---------------------------
void setup() {
    Serial.begin(115200);
    Blynk.begin(auth, ssid, pass);
}

// ---------------------------
// MAIN LOOP
// ---------------------------
void loop() {
    Blynk.run();

    if (!turnOn || queueSize == 0) return;

    // wait for button press
    if (digitalRead(buttonPin) == LOW && !buttonPressed) {
        buttonPressed = true;
        delay(50); // debounce

        if (digitalRead(buttonPin) == LOW && queueSize > 0) {
            String word = queue[0];
            Serial.print("Button pressed. Displaying: ");
            Serial.println(word);

                    // Iterate letters safely
        for (int i = 0; i < word.length();) {
            char c = word[i];
            int charSize = 1;

            if ((c & 0x80) != 0) {
                if ((c & 0xE0) == 0xC0) charSize = 2;
                else if ((c & 0xF0) == 0xE0) charSize = 3;
                else if ((c & 0xF8) == 0xF0) charSize = 4;
            }

            String letter = word.substring(i, i + charSize);
            Serial.println(letter);
            lightLetter(letter, 500);
            delay(200);
            i += charSize;
        }
            unsigned char * currentFlow = gen_spi2();
            dequeue();
        }
    }

    // reset when button released
    if (digitalRead(buttonPin) == HIGH) {
        buttonPressed = false;
    }
}
