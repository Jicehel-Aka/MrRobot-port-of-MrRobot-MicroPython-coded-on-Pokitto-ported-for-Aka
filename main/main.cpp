// main.cpp — Point d'entree de l'app AKA "MrRobot" (jeu MicroPython
// autonome, exactement comme un jeu C++ classique du point de vue du
// loader AKA : sa propre entree, son propre port_manifest.json, sa
// propre icone). Reutilise le composant micropython/aka PARTAGE, non
// modifie -- seule la configuration ci-dessous (identifiant, script,
// credits) differe du lanceur generique "micropython".
//
// GABARIT : pour un nouveau jeu MicroPython autonome, copier ce fichier
// et n'adapter QUE les lignes marquees "// A ADAPTER :".
#include "gb_core.h"
#include "gb_graphics.h"
#include "gb_audio_player.h"
#include "core/input.h"
#include "aka_runtime/aka_runtime.h"
#include "aka_hal.h"                 // aka_boot_init / aka_boot_exec / aka_hal_set_*

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_heap_caps.h"
#include "esp_log.h"

// Instances globales uniques, partagees avec aka_runtime, input et aka_hal.
gb_core         g_core;
gb_graphics     gfx;
gb_audio_player g_audio_player;

static const char *TAG = "mrrobot_aka";

// A ADAPTER : chemin du script principal DE CE JEU (pas le dossier
// generique /sdcard/py/) -- propre sous-dossier sur la carte SD, comme
// tout autre jeu AKA.
#define AKA_MAIN_PY "/sdcard/mrrobot/main.py"

// Taille du tas GC MicroPython : de preference en PSRAM (large), repli en RAM
// interne si la PSRAM est absente. Stable, ne pas modifier.
#define AKA_HEAP_PSRAM (2 * 1024 * 1024)
#define AKA_HEAP_INTERNAL (128 * 1024)

static void audio_mix_task(void *) {
    for (;;) {
        g_audio_player.pool();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

extern "C" void app_main(void) {
    // 1) Materiel : ecran, bus I2C/ADC, expander, CARTE SD (montee ici), audio.
    // Stable, ne pas modifier.
    g_core.init();
    gfx.set_backlight_percent(80);
    gfx.set_refresh_rate(60);
    input_init();

    // 2) Socle AKA -- A ADAPTER : identifiant court DE CE JEU (determine
    // son propre dossier /sdcard/<id>/, distinct du dossier generique
    // /sdcard/micropython/ du lanceur multi-scripts).
    akaRuntime.begin("mrrobot");
    static auto applyVolume = [](uint8_t musicVol, uint8_t /*sfxVol*/) {
        g_audio_player.set_master_volume((uint8_t)((uint16_t)musicVol * 200 / 100));
    };
    akaRuntime.setVolumeChangedCallback(applyVolume);

    // A ADAPTER : commandes et credits DE CE JEU (un script .py peut
    // toujours les redefinir via aka.set_controls()/aka.set_credits() --
    // ceci n'est qu'une valeur par defaut avant que le script ne
    // s'execute).
    //
    // BUG TROUVE ET CORRIGE : ces chaines etaient d'abord codees en dur en
    // francais -- un joueur en anglais/allemand/etc. aurait quand meme vu
    // du francais, contrairement a la convention utilisee sur tous les
    // autres jeux (cles de traduction + sdcard_files/<jeu>/lang/*.json).
    // Le menu systeme appelle translate() sur chaque entree (voir
    // aka_runtime.cpp), donc passer des CLES ici fonctionne exactement
    // comme pour un jeu Arduboy/C++.
    // BUG TROUVE ET CORRIGE : "CTRL_JUMP" affirmait a tort qu'on saute
    // par-dessus les fusees -- verifie dans le vrai code source du jeu
    // (main.py) : robotY est une CONSTANTE, jamais modifiee -- il n'y a
    // AUCUN saut. Seuls comptent : gauche/droite pour esquiver, et A pour
    // demarrer/recommencer.
    static const char *const kControls[] = {
        "CTRL_MOVE",
        "CTRL_START",
        "CTRL_MENU_SHORT",
        "CTRL_MENU_LONG",
        "CTRL_QUIT",
        nullptr
    };
    aka_hal_set_controls(kControls, 5);
    aka_hal_set_credits("Mr Robot", "Hannu Viitala", "MIT",
                        "github.com/haviital/MrRobot");

    g_audio_player.set_master_volume((uint8_t)((uint16_t)akaRuntime.getMusicVolume() * 200 / 100));
    xTaskCreatePinnedToCore(audio_mix_task, "AudioMixTask", 4096, nullptr, 5, nullptr, 1);

    // 3) Tas GC MicroPython. Stable, ne pas modifier.
    size_t heap_size = AKA_HEAP_PSRAM;
    void  *heap = heap_caps_malloc(heap_size, MALLOC_CAP_SPIRAM);
    if (!heap) {
        heap_size = AKA_HEAP_INTERNAL;
        heap = heap_caps_malloc(heap_size, MALLOC_CAP_8BIT);
    }
    ESP_LOGI(TAG, "MicroPython : tas = %u octets @ %p", (unsigned)heap_size, heap);

    // 4) Demarrage de la VM + execution DIRECTE du script de ce jeu --
    // PAS de menu de selection : ce jeu se comporte comme n'importe quel
    // autre jeu AKA autonome, il boote directement dedans. Le script
    // Python peut lui-meme, s'il le souhaite, appeler aka.run_file() pour
    // basculer vers un autre script (menu maison optionnel), mais ce
    // n'est plus le comportement PAR DEFAUT comme dans le lanceur
    // generique "micropython".
    aka_boot_init(heap, heap_size);
    ESP_LOGI(TAG, "Execution de %s", AKA_MAIN_PY);
    aka_boot_exec(AKA_MAIN_PY);

    // 5) Si le script rend la main (retour de sa boucle), on continue de servir
    // le menu systeme pour que RUN+MENU ramene toujours au loader.
    // Stable, ne pas modifier.
    ESP_LOGW(TAG, "Le script principal s'est termine ; boucle de veille.");
    while (true) {
        input_poll(g_keys);
        akaRuntime.update(g_keys);
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

