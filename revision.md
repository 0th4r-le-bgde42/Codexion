# 🛡️ Codexion : Le Master Cheat Sheet de Soutenance

Ce document contient tout ce qu'il faut pour justifier l'architecture, la synchronisation mémoire et l'algorithmique du projet, afin de répondre aux correcteurs les plus exigeants.

---

## 🗺️ 1. Le Chemin du Programme (Flux d'Exécution)

Voici la chronologie exacte de la vie de ton processus, du lancement à la mort :

    [ MAIN (Thread Principal) ]
       │
       ├─► 1. parsing() : Validation stricte des 8 arguments obligatoires.
       │
       ├─► 2. init_dongles() & init_coders() : 
       │      - Allocation mémoire (malloc).
       │      - Création des Mutex (1 par dongle + write_mutex + stop_mutex).
       │      - Création des Cond Vars (1 par dongle).
       │      - Allocation des files d'attente (Min-Heap) pour le scheduler.
       │
       ├─► 3. launch_sim() : LE GRAND DÉPART
       │      │
       │      ├─► pthread_create() x N ──► [ THREADS CODEURS (1 à N) ]
       │      │                              │
       │      │                              ├─► coder_routine() boucle à l'infini :
       │      │                              │    1. Vérifie stop_mutex.
       │      │                              │    2. manage_dongle_in() (Anti-Deadlock).
       │      │                              │    3. compilation_step() + smart_sleep().
       │      │                              │    4. manage_dongle_out() (Cond Broadcast).
       │      │                              │    5. Debug & Refactor + smart_sleep().
       │      │                              │
       │      └─► pthread_create() x 1 ──► [ THREAD MONITOR ]
       │                                     │
       │                                     ├─► monitor_routine() boucle :
       │                                          1. Vérifie quota de compilations.
       │                                          2. check_death() : Compare timestamps.
       │                                          3. Si mort : wake_up_call() + print "burned out".
       │
       ├─► 4. cleanup_sim() : Le Main attend.
       │      - pthread_join() du Monitor.
       │      - pthread_join() de tous les Codeurs.
       │
       └─► 5. free_all() : Destruction des Mutex/Cond, Free mémoires (aucune fuite), EXIT 0.



---

## 🔬 2. Analyse Chirurgicale des Mutex (L'Horlogerie Interne)

Dans ton architecture, les mutex ne servent pas qu'à faire joli, ils protègent contre des **Data Races** mortelles (corruption mémoire due à des accès concurrents).

### 🔒 Le `stop_mutex` (La barrière de mémoire)
* **Que protège-t-il ?** Les variables `simulation_stop` et `stop_reason`.
* **Pourquoi est-il vital ?** Le thread Monitor est le seul à *écrire* `simulation_stop = 1`. Mais **tous** les threads codeurs *lisent* cette variable en permanence. Sans ce mutex, le processeur ou le compilateur (optimisation) pourrait stocker la valeur dans le cache local du thread. Le thread codeur ne verrait **jamais** que le Monitor a passé la variable à 1 en RAM et tournerait à l'infini. Le mutex force une relecture en RAM.

### 📝 Le `write_mutex` (La double protection)
* **1. Sérialisation de l'I/O :** `printf` n'est pas "atomique". Sans `write_mutex`, le système d'exploitation pourrait couper le thread A en plein milieu de son message pour laisser le thread B écrire. Le sujet exige qu'une ligne ne soit jamais mélangée avec une autre.
* **2. Protection des statistiques vitales :** Dans `compilation_step`, le `write_mutex` protège la mise à jour de `last_compile_start` et `compiles_done`. Sans cela, le Monitor pourrait lire une valeur corrompue dans `check_death()` exactement au moment où l'octet est réécrit.

### 🎛️ L'Architecture "1 Mutex + 1 Cond par Dongle"
* **1 Mutex par Dongle (Granularité fine) :** Plutôt qu'un énorme mutex global qui bloquerait toute la table, chaque dongle a son verrou. Si le codeur 1 prend son dongle, il ne bloque pas le codeur 50 à l'autre bout de la pièce.
* **1 Cond par Dongle (Anti-Thundering Herd) :** Si un codeur pose un dongle, un `pthread_cond_broadcast` global réveillerait *tous* les threads de la simulation, qui crameraient du CPU pour se rendormir aussitôt. En liant la `cond` au dongle, on ne réveille **que** les threads qui attendent spécifiquement ce dongle.



---

## ⚙️ 3. Gestion des Ressources : Deadlock, Scheduler et Cooldown

### A. La Prévention du Deadlock (Interblocage)
* **Le Danger :** Si tous les codeurs prennent leur dongle gauche en même temps, ils attendent tous à l'infini leur dongle droit.
* **Ta Solution (`manage_dongle_in`) :** L'acquisition asymétrique. Les codeurs pairs prennent la droite puis la gauche, les impairs l'inverse. Cela brise la condition d'attente circulaire (Conditions de Coffman).

### B. L'Ordonnanceur (Scheduler EDF/FIFO)
* **Le Danger :** La famine (Starvation). Un codeur ne peut jamais compiler car d'autres lui passent devant.
* **Ta Solution (`heap.c`) :** Une file de priorité (Min-Heap) implémentée sur mesure.
    * **FIFO :** Le tas trie par ordre d'arrivée (`request_time`).
    * **EDF :** Le tas trie par urgence (`last_compile_start + time_to_burnout`).

### C. Le Cooldown des Dongles (Attente hybride)
* **La règle :** Un dongle relâché est indisponible pendant `dongle_cooldown` ms.
* **La Solution (`take_dongle`) :** Si un codeur est premier de la file mais que le cooldown n'est pas fini, il fait une *attente active intelligente* : 
  `pthread_mutex_unlock` ➔ `usleep(500)` (micro-sieste) ➔ `pthread_mutex_lock`.
  Il relâche le verrou pour laisser les autres interagir avec la file, dort un instant, puis revérifie.

---

## ⏱️ 4. La Survie et la Sortie (Monitor et Wake-up call)

* **La Mort Ultra-Précise (< 10ms) :** Le sujet exige un affichage de la mort en moins de 10ms. Le Monitor tourne toutes les 1ms (`usleep(1000)`). Les codeurs utilisent `smart_sleep` (micro-siestes de 500µs) et vérifient constamment le `simulation_stop`.
* **Le "Wake Up Call" (La Sortie Propre) :** Si le Monitor détecte la fin, il fait un `pthread_cond_broadcast` sur **tous** les dongles. Les threads bloqués dans `pthread_cond_wait` se réveillent, voient que la simulation est finie, lâchent les mutex et se terminent proprement.

---

## ⚡ 5. La "Quick Live Modif" (Recode Instruction)

Lors de la soutenance, on peut te demander une modification rapide pour vérifier ta compréhension. **Scénario classique : prioriser les codeurs par leur ID (les plus petits passent en premier) au lieu de leur deadline en mode EDF.**

Ouvre `heap.c` et modifie uniquement la fonction `compare_coders` :

**APRÈS (La version modifiée en live) :**
```c
int compare_coders(t_coder *a, t_coder *b, t_config *config)
{
    if (config->scheduler_type == FIFO)
        return (a->request_time < b->request_time);
    
    /* Live Modif: L'ID le plus petit gagne la priorité absolue */
    return (a->id < b->id); 
}
```

**Ce que tu dois dire au correcteur :**
> *"J'ai modifié la condition de tri de mon arbre (Min-Heap). Maintenant, la fonction `sift_down` et `heap_push` font remonter l'ID le plus petit à la racine de l'arbre (index 0). Le codeur 1 passera donc toujours devant le codeur 5, créant expérimentalement une famine (starvation) pour les grands IDs."*

---

## ⚔️ 6. FAQ : Les Questions Pièges du Correcteur

| Question | Ta Réponse |
| :--- | :--- |
| **C'est quoi un Data Race ?** | C'est quand deux threads lisent et écrivent la même variable en même temps, corrompant la donnée. Je l'évite avec des `pthread_mutex_lock/unlock` sur chaque ressource partagée. |
| **Pourquoi un `while` autour de `pthread_cond_wait` ?** | Pour gérer les *"Spurious Wakeups"* (réveils abusifs de l'OS). Le thread doit revérifier s'il est vraiment le premier de la Heap, si le dongle est libre, et si le cooldown est respecté avant d'avancer. |
| **Pourquoi `write_mutex` est si important ?** | Il sérialise l'output. Sans lui, les messages se mélangeraient sur la même ligne (ex: `120 1 is comp20 2 is debuggingiling`), ce qui est strictement interdit. |
| **Que se passe-t-il si un thread meurt PENDANT qu'un autre print ?** | Le Monitor met le flag de mort, puis attend le `write_mutex` pour afficher 'burned out'. Dès qu'il l'obtient et affiche la mort, la condition `!coder->config->simulation_stop` dans `print_log` empêchera tout autre message de s'afficher. La mort sera toujours le dernier message. |