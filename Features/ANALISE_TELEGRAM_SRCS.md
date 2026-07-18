# Análise Completa dos Sources de Todos os Painéis
## GhostSystems Free Fire — Mapeamento de Features e Bypass

> **Objetivo deste documento**: Mapear todas as features, UI e lógicas de bypass existentes no repositório para facilitar a reutilização em painéis futuros. Sem código-fonte, apenas descrição funcional.

---

## Sumário

1. [Visão Geral da Arquitetura](#1-visão-geral-da-arquitetura)
2. [Painel AXL (AxlMods)](#2-painel-axl-axlmods)
3. [Painel SYZEN](#3-painel-syzen)
4. [Painel BRUXO (Hush)](#4-painel-bruxo-hush)
5. [Painel GOAT CHEATS (gsmodz)](#5-painel-goat-cheats-gsmodz)
6. [Painel GUJU MODS](#6-painel-guju-mods)
7. [Tabela Completa de Features do Menu (UI)](#7-tabela-completa-de-features-do-menu-ui)
8. [Tabela Completa de Técnicas de Bypass](#8-tabela-completa-de-técnicas-de-bypass)
9. [Módulos Compartilhados Independentes de Painel](#9-módulos-compartilhados-independentes-de-painel)
10. [Guia de Reutilização — Como Implementar Features Existentes no Seu Painel](#10-guia-de-reutilização--como-implementar-features-existentes-no-seu-painel)

---

## 1. Visão Geral da Arquitetura

### Estrutura de Diretórios

```
GhostSystemsPainel/
├── Menu.cpp / Menu_fixed.cpp / Menu_append.cpp / MenuExtras.cpp   ← UI principal (tabs ImGui)
├── NetworkProxy.h / NetworkProxy.cpp                                ← Proxy de rede compartilhado
├── MemoryCloak.h / MemoryCloak.cpp                                  ← Cloak de memória / arquivos
├── BypassManager.h / BypassManager.cpp                              ← Gerenciador de 40 técnicas de bypass
├── BypassLoginSDK.h / BypassLoginSDK.cpp                            ← Bypass do SDK de login (ffantihack)
├── And64InlineHook.hpp / And64InlineHook.cpp                        ← Motor de inline hook ARM64 (A64HookFunction)
├── Il2CppHelper.h / IL2CppDumper.h                                 ← Helpers para Unity IL2CPP
├── OffsetResolver.h / TouchReader.h / Obfuscator.h                 ← Utilitários de infraestrutura
│
└── Features/
    ├── SRCS_PAINEIS/        ← Fontes dos painéis
    │   ├── AXL/
    │   │   ├── AxlMods/           (Hook.h, gui.hpp, cheat.cpp, And64InlineHook.*)
    │   │   ├── Unity/             (Il2Cpp, EspManager, Vector3, Color, etc.)
    │   │   ├── KittyMemory/       (scanner, memória read/write)
    │   │   ├── xdl/               (carregador dinâmico XDL)
    │   │   └── oxorany/           (obfuscação XOR de strings)
    │   │
    │   ├── BRUXO/
    │   │   ├── ZygModule/         (zygisk.h, cheat.cpp, layout.h, And64InlineHook.*)
    │   │   ├── Hush/              (bypass.cpp, aimbot.h, ESP.h, Chams.h, particulas.h)
    │   │   ├── KittyMemory/
    │   │   ├── Misc/imgui/        (imagens, fontes, backend completo do ImGui)
    │   │   └── Misc/update/xdl/
    │   │
    │   ├── SYZEN/
    │   │   ├── ZygModule/         (zygisk.h, cheat.cpp, And64InlineHook.*)
    │   │   ├── Misc/imgui/
    │   │   ├── Misc/update/xdl/
    │   │   ├── Misc/Unity/
    │   │   ├── eLogin/            (sistema de login/licença criptografada)
    │   │   └── Login/             (auth, libcurl, utils de licença)
    │   │
    │   ├── GOAT CHEATS/
    │   │   ├── zygisk/            (cheat.cpp, BatteryInfo.h/.cpp, ByPass старт ТУТ)
    │   │   ├── gsmodz/            (bypass.cpp próprio)
    │   │   └── others/
    │   │
    │   └── GUJU MODS/
    │       ├── zygisk.hpp
    │       ├── xdl/
    │       └── Struct/
    │
    └── (módulos compartilhados independentes de painel)
        ├── AimLock.h / AimLock.cpp
        ├── AimMagnet.h / AimMagnet.cpp
        ├── EnemyPull.h / EnemyPull.cpp
        ├── GhostHack.h / GhostHack.cpp
        ├── Magnet.h / Magnet.cpp
        ├── NoRecoil.h / NoRecoil.cpp
        ├── Teleport.h / Teleport.cpp
        ├── UpDown.h / UpDown.cpp
        ├── FakeLag.h / FakeLag.cpp
        ├── SharedAccess.h
        ├── IFeature.h
        ├── ChamsManager.cpp / ChamsGlHook.cpp
        ├── BypassManager.* / BypassLoginSDK.*
        └── Entity.h
```

### Padrão Comum de Todos os Painéis

- **Injeção**: Via Zygisk (Magisk) → módulo nativo injetado no processo do jogo.
- **Hooking**: `A64HookFunction` (Substrate) para inline hooks ARM64.
- **UI**: ImGui rodando sobre OpenGL ES 3 via `imgui_impl_opengl3` + `imgui_impl_android`.
- **Carregamento de símbolos**: XDL (custom dynamic linker) para encontrar funções em libraries carregadas.
- **Obfuscação**: Macro `OBFUSCATE()` para esconder strings no binário.
- **Anti-dedução**: Memory Cloak (filtra `/proc/self/maps`) + bypasses por painel.

---

## 2. Painel AXL (AxlMods)

### Caminho dos Sources
```
Features/SRCS_PAINEIS/AXL/
├── AxlMods/
│   ├── Hook.h              → TODAS as flags de features (bools) + struct cfg
│   ├── gui.hpp             → Setup do ImGui (EGL, OpenGL, input Android, fontes)
│   ├── cheat.cpp           → Entry point, chamada de Init e hooks principais
│   ├── And64InlineHook.hpp / .cpp
│   └── ... (outros .cpp de feature)
├── Unity/                  → Definições Il2Cpp/Unity (Il2Cpp.h, EspManager.h, Vector3.hpp, Color.h)
├── KittyMemory/            → Scanner, MemoryPatch, KittyArm64
├── xdl/                    → XDL completo (carregador dinâmico)
└── oxorany/                → Obfuscação XOR
```

### Features Implementadas (via UI em `gui.hpp` + flags em `Hook.h`)

| Feature | Tipo | Efeito |
|---------|------|--------|
| ESP (Box, Line, Health, Name, Distance, Skeleton) | Checkboxes / combos | Desenha caixas, linhas, barras de vida, nomes, distância e esqueleto sobre os jogadores |
| 3D Box / Box 3D | Combo | Alterna entre caixa 2D e caixa 3D (projeção de pontos do bounding box) |
| Line ESP (3 modos) | Combo | Três estilos de linha (topo→jogador, jogador→jogador, etc.) |
| Aimbot (Tradicional / Aimlock) | Combo + checkboxes | Mira automática com FOV, delay, transição cabeça, força/curva |
| Silent Aim | Checkbox | Redireciona Raycast/Physics para acertar sem mover a câmera |
| Mira Magnética / Puxa Inimigo | Checkbox | Teleporta o inimigo para a mira ( EnemyPull embutido ) |
| No Recoil | Checkbox | Zera recuo da arma a cada tiro |
| SpeedHack (FastReload / FastSwitch) | Checkbox | Acelera recarga e troca de armas |
| Camera Free / Speed Camera | Checkbox | Permite mover a câmera livremente ou aumentar velocidade |
| Double Gun | Checkbox | Permite atirar com duas armas simultaneamente |
| Car (veículo) | Checkbox | Habilita funcionalidades de veículo (velocidade/teleporte?) |
| Reset Guest | Checkbox + ação | Reseta conta guest deletando arquivo de dados |
| Ghost / NeedSendMessage | Hook de rede | Suprime mensagens de rede (stealth mode) |
| Sniper Alert | Checkbox | Alerta quando sniper está mirando em você |
| Aim360 | Checkbox | Mira em 360° (ignora FOV?) |
| Noscope | Checkbox | Mira sem escopo (snap instantâneo) |
| Particulas / Efeitos | Partículas | Efeitos visuais customizados no overlay |

### Bypass Específico do AXL

- **Obfuscação de strings**: Todas as referências a bibliotecas e classes sensíveis usam `OBFUSCATE("string")`.
- **Reset de guest com remoção de arquivo**: Deleta `/storage/emulated/0/com.garena.msdk/guest100067.dat` para resetar identidade banida.
- **Ghost Network Mode**: Hook em `NeedSendMessage` retorna `true` suprimindo envio de pacotes.

---

## 3. Painel SYZEN

### Caminho dos Sources
```
Features/SRCS_PAINEIS/SYZEN/
├── ZygModule/
│   ├── zygisk.h            → Entry point Zygisk
│   ├── cheat.cpp           → Inicialização, hooks de rede e Il2Cpp
│   ├── And64InlineHook.hpp / .cpp
│   ├── layout.h            → Constantes de layout UI
│   ├── telaonline.h        → Indicador "online" na tela
│   ├── theme_config.cpp    → Cores e temas da UI
│   ├── particles.h / .cpp  → Efeitos de partículas
│   └── sd.h / particulas.h
├── Misc/
│   ├── imgui/              → Backend completo do ImGui (igual outros painéis)
│   ├── update/xdl/         → XDL completo
│   ├── Unity/              → Il2Cpp.h/.cpp, EspManager, Vector3, Quaternion, Color, etc.
│   └── (imports.h, obfuscate.h, tools.hpp, imguiconfig.h)
└── eLogin/                 ← SISTEMA DE LOGIN/LICENÇA COMPLETO
    ├── Login.h
    ├── Tools.h / Tools.cpp
    ├── Log.h / Log.cpp
    ├── strEnc.h / obfuscate.h
    ├── json.hpp
    ├── Includes.h
    ├── LicenseTools.h
    └── (dependências: libcurl, json)
```

### Features do SYZEN

| Feature | Efeito |
|---------|--------|
| ESP (2D/3D boxes, health, distance, skeleton) | Overlay de jogadores com projeção de tela |
| Aimbot | Mira automática com alvo, FOV e suavização |
| Auto-aim + Auto-attack | Mira e atira automaticamente quando alvo está na mira |
| SpeedHack | Modificador de velocidade de movimento (via Time.timeScale ou Player.Move) |
| Partículas / Efeitos visuais | Efeitos custom overlay |
| Telinha Online (status) | Mostra status do painel na tela (conectado/desconectado) |
| Sistema de Login/Licença (eLogin) | Verificação de licença criptografada, conexão com servidor de auth, máquina de estados de login (log, tools, json, libcurl) |

### Bypass Específico do SYZEN

- **Delay de carregamento de biblioteca**: Espera `libil2cpp.so` carregar completamente (`while (!getLibraryLoaded("libil2cpp.so")) sleep(3)`) antes de aplicar hooks, evitando detecção por ordem de carregamento.
- **Hook padrão de `eglSwapBuffers`**: Usado para renderizar o overlay ImGui a cada frame (também presente nos outros painéis).

---

## 4. Painel BRUXO (Hush)

### Caminho dos Sources
```
Features/SRCS_PAINEIS/BRUXO/
├── ZygModule/
│   ├── zygisk.h
│   ├── cheat.cpp           → Aplica bypass + instala aimbot/ESP/speedHack/chams
│   ├── And64InlineHook.hpp / .cpp
│   ├── layout.h
│   ├── telaonline.h
│   ├── theme_config.cpp
│   └── particles.h / .cpp
├── Hush/
│   ├── bypass.cpp          ← BYPASS PRINCIPAL (125+ hooks + JNI + dlopen/open/fopen/ptrace)
│   ├── aimbot.h / aimbot.cpp
│   ├── ESP.h / ESP.cpp
│   ├── Chams.h / Chams.cpp
│   └── particulas.h
├── KittyMemory/
├── Misc/
│   ├── imgui/              → Backend ImGui completo com fonts customizadas (gsicons.h)
│   └── update/xdl/
└── Android.mk / Application.mk  (makefile standalone)
```

### Features do BRUXO

| Feature | Efeito |
|---------|--------|
| ESP (Box, Linha, Health, Nome, Distância, Esqueleto 2D/3D) | Overlay completo sobre jogadores |
| Chams / Wallhack | Renderização colorida através de paredes (materiais Il2Cpp) |
| Aimbot (FOV, suavização, seleção de osso) | Mira automática com alvos e velocidade ajustável |
| SpeedHack | Modifica velocidade de movimento |
| Teleporte / Free Move | Teleporta jogador ou permite movimento livre no cenário |
| Partículas customizadas | Efeitos visuais overlay |
| Bypass integrado (Hush/bypass.cpp) | 125+ hooks + JNI + filtro de arquivos |

### Técnicas de Bypass do BRUXO/Hush (detalhadas)

| Técnica | Funções Hookadas | Efeito |
|---------|-----------------|--------|
| **Interceptação de libc** | `ptrace`, `fopen`, `open`, `dlopen`, `getuid`, `readlink` | Bloqueia debugger, esconde bibliotecas de anti-cheat, bloqueia leitura de `/proc/self/maps` |
| **Filtro de abertura de arquivo** | `fopen` / `open` | Retorna NULL/-1 para paths contendo: "cheat", "mod", "xposed", "frida", `/proc/self`, e nomes de libs Garena (`libanogs.so`, `libanort.so`, `libil2cpp.so`, `libunity.so`, `libmain.so`, `libcrashlytics.so`, `libFirebaseCppApp-11_8_1.so`, `libff_voice_engine.so`, `libFFVoiceMagicVoiceEngine.so`, `SH-Gpp2`, `liblongs.so`) |
| **Bloqueio de dlopen** | `dlopen` | Retorna NULL para bibliotecas suspeitas ("libmod", "libcheat", "libxposed", "libfrida" + nomes acima) |
| **Spoof de UID** | `getuid` | Retorna 10000 (usuário não-root) |
| **JNI Anti-Debug** | `ApplicationInfo.isDebuggable()`, `com.garena.android.buildconfig.DEBUG`, `PackageManager.getPackageInfo()` | Retorna `false`/`nullptr` — app parece não-debuggable |
| **Offset Inline Hooks Agressivos** | 125+ offsets em 8 bibliotecas | Hooks sem função (NOP) em `libanogs.so` (5 offsets), `libil2cpp.so` (1), `libunity.so` (5), `libFirebaseCppApp` (3), `libcrashlytics-common.so` (1), `SH-Gpp2` (1), `libFFVoiceMagicVoiceEngine.so` (59 offsets!), `libff_voice_engine.so` (39 offsets) |

> **Nota**: BRUXO é o painel com a camada de bypass mais agressiva do repositório (125+ inline hooks).

---

## 5. Painel GOAT CHEATS (gsmodz)

### Caminho dos Sources
```
Features/SRCS_PAINEIS/GOAT CHEATS/
├── zygisk/
│   ├── zygisk.h
│   ├── cheat.cpp           → Entry point + hooks padrão
│   ├── And64InlineHook.hpp / .cpp
│   ├── layout.h
│   ├── telaonline.h
│   ├── particulas.h
│   ├── BatteryInfo.h / .cpp
│   └── (estrutura similar a SYZEN/BRUXO)
├── gsmodz/
│   └── bypass.cpp          ← BYPASS DO GSmodz (versão simplificada do BRUXO)
└── others/                 → Unity + XDL + obfuscate (shared infra)
```

### Features do GOAT CHEATS

| Feature | Efeito |
|---------|--------|
| ESP padrão | Overlay de jogadores |
| Aimbot | Mira automática |
| SpeedHack | Velocidade aumentada |
| Partículas overlay | Efeitos visuais |
| Indicador de bateria | Exibe nível de bateria na UI (BatteryInfo) |
| Bypass simplificado | Versão reduzida do bypass do BRUXO |

### Bypass do GOAT CHEATS (gsmodz/bypass.cpp)

| Função | Efeito |
|--------|--------|
| `ptrace` | Retorna -1 (bloqueia debugger) |
| `fopen` / `open` | Bloqueia paths: "cheat", "mod", "xposed", "frida", `/proc/self/` |
| `dlopen` | Retorna NULL para libs suspeitas |
| `getuid` | Retorna 10000 (spoof de não-root) |
| `readlink` | Retorna -1 para `/proc/self/maps` e `/proc/self/exe` |
| JNI `isDebuggable` / `DEBUG` / `getPackageInfo` | Retorna `false`/`nullptr` sempre (não condicional) |

> **Diferença para BRUXO**: GOAT CHEATS tem uma lista de bloqueio menor (sem os nomes específicos de libs Garena), e o JNI bypass é sempre ativo (não usa flag `bypass_enabled`).

---

## 6. Painel GUJU MODS

### Caminho dos Sources
```
Features/SRCS_PAINEIS/GUJU MODS/
├── zygisk.hpp              → Entry point Zygisk + API de hooks (PLT, JNI)
├── xdl/                    → XDL completo
└── Struct/
    └── Unity.h             → Definições Unity compactas
```

### Características do GUJU MODS

| Aspecto | Detalhe |
|---------|---------|
| **Abordagem** | Painel experimental / sandbox. Usa Zygisk puro com API nativa (não apenas Dobby). |
| **Injeção** | `zygisk::ModuleBase` com `preAppSpecialize`/`preServerSpecialize` |
| **Auto-unload** | `Option::DLCLOSE_MODULE_LIBRARY` — a biblioteca do módulo é `dlclose`-ada após a especialização, deixando apenas os trampolines de hook na memória. A biblioteca some do `/proc/self/maps`. |
| **Denylist Unmount** | `Option::FORCE_DENYLIST_UNMOUNT` — força desmonte da denylist do Magisk no namespace de montagem do processo. |
| **PLT Hooks nativos** | `Api::pltHookRegister()` + `pltHookCommit()` — hook por device+inode ELF (mais stealth que `dlsym`). |
| **Hook JNI nativo** | `Api::hookJniNativeMethods()` — hook de métodos JNI nativos por nome/assintura. |
| **fake_dlfcn** | Implementação própria de `dlopen`/`dlsym` parseando ELF diretamente via `/proc/self/maps` + `mmap` (bypass para hooks de anti-cheat em `dlopen`/`dlsym` do libc). |
| **Offsets randomizados** | `getRealOffset()` adiciona ruído aleatório (`rand() % 0x1000`) aos endereços base para evitar scans determinísticos. |
| **Patches hex AArch64** | `patchLib()` injeta bytes diretamente em offsets de bibliotecas (ex.: `mov x0, #0; ret` para retornar false). |
| **Emulator Bypass** | Hook em `COW.GameFacade.IsEmulator()` retorna `false` (anti-emulador). |

---

## 7. Tabela Completa de Features do Menu (UI)

> Fonte primária: `Menu.cpp` (canônico, completo). Features adicionais podem existir nos arquivos de origem de cada painel e não estar expostas na UI principal.

### 7.1 Controle Mestre

| # | UI Label | Variável | Tipo | Efeito | Linha (Menu.cpp) |
|---|----------|----------|------|--------|------------------|
| 1 | Ativar Painel | `masterSwitch` | Checkbox | Liga/desliga scanner e todas as features | 283 |
| 2 | Minimizar (_) | `panelMinimized` | Botão | Minimiza painel para ícone flutuante GS | 484 |

### 7.2 Tab AIMBOT (linhas ~508-543)

| # | Feature | Variável | Tipo | Efeito | Linha |
|---|---------|----------|------|--------|-------|
| 3 | Ativar Aimbot | `aimbotEnabled` | Checkbox | Habilita motor de aimbot | 509 |
| 4 | Modo | `aimbotMode` | Combo (0=Tradicional/Ao Atirar, 1=Aimlock/Sempre) | Define modo de funcionamento | 511-512 |
| 5 | Mostrar FOV | `aimbotDrawFov` | Checkbox | Desenha círculo de FOV na tela | 513 |
| 6 | Mirar em Aliados | `aimbotTargetAllies` | Checkbox | Permite mirar em teammates | 514 |
| 7 | Ignorar Derrubados | `aimbotIgnoreKnocked` | Checkbox | Ignora jogadores DBNO/derrubados | 515 |
| 8 | Silent Aim | `aimbotSilentAim` | Checkbox | Redireciona Raycast/Physics para acertar sem mover câmera | 516 |
| 9 | Mira Magnética (Puxa Inimigo) | `aimbotMagnetic` | Checkbox | Puxa inimigo magneticamente para a mira (teleporte) | 517 |
| 10 | Raio do FOV | `aimbotFov` | Slider (10-500px) | Raio do campo de visão do aimbot | 518 |
| 11 | Atraso/Delay (ms) | `aimbotTimeMs` | Slider (0-300ms) | Delay antes do aim snap (Rage <50, Safe >50) | 519 |
| 12 | Tempo p/ Cabeça (ms) | `aimbotTransitionTimeMs` | Slider (0-2000ms) | Tempo de transição peito → cabeça | 529 |
| 13 | Força/Curva | `aimbotTransitionCurve` | Slider (1.0-10.0) | Curva de aceleração da transição | 535 |
| 14 | No Recoil | `noRecoilEnabled` | Checkbox | Remove recuo da arma via hook | 539 |

### 7.3 Tab ESP (linhas ~546-592)

| # | Feature | Variável | Tipo | Efeito | Linha |
|---|---------|----------|------|--------|-------|
| 15 | Ativar ESP | `espEnabled` | Checkbox | Master toggle do ESP | 547 |
| 16 | ESP Box | `espBox` | Checkbox | Desenha retângulo ao redor dos jogadores | 549 |
| 17 | Modo Box | `espBoxMode` | Combo (0=Sólido, 1=Outline) | Estilo da caixa ESP | 551-552 |
| 18 | ESP Vida | `espHealth` | Checkbox | Barra de vida ao lado da caixa | 554 |
| 19 | ESP Nome | `espName` | Checkbox | Nome do jogador acima da cabeça | 555 |
| 20 | ESP Distância | `espDistance` | Checkbox | Distância até o jogador | 556 |
| 21 | ESP Linha | `espLine` | Checkbox | Linha do topo da tela até o jogador | 557 |
| 22 | Esqueleto (Bones) | `espSkeleton` | Checkbox | ESP de esqueleto completo (cabeça, pescoço, coluna, braços, pernas) | 558 |
| 23 | Chams | `chamsEnabled` | Checkbox | Override de material (glow/chams via Il2Cpp) | 559 |
| 24 | Máxima (Distância) | `espMaxDistance` | Slider (10-300m) | Distância máxima de renderização do ESP | 563 |
| 25 | Ativar Wall-Check | `featureConfig.wallCheckEnabled` | Checkbox | Habilita verificação de parede global | 567 |
| 26 | Método (Raycast) | `featureConfig.wallCheckMethod` | Combo (0-3) | Physics.Raycast / Linecast / CheckSphere / Custom | 569-575 |
| 27 | Testar múltiplos pontos | `wallCheckMultiplePoints` | Checkbox | Testa visibilidade em cabeça/pescoço/peito | 579 |
| 28 | Origem do jogador local | `wallCheckLocalOrigin` | Combo (0=Pé, 1=Peito, 2=Cabeça) | Define ponto de origem do raycast local | 581-586 |

### 7.4 Tab CONFIGURAÇÕES (linhas ~594-638)

| # | Feature | Variável | Tipo | Efeito | Linha |
|---|---------|----------|------|--------|-------|
| 29 | MODO STREAM (Invisível p/ captura) | `featureConfig.streamModeEnabled` | Checkbox | Oculta overlay de captura de tela/stream | 597 |
| 30 | Toggle 3 Dedos | `featureConfig.threeFingerToggleEnabled` | Checkbox | Mostra/esconde painel com gesto de 3 dedos | 598 |
| 31 | Ativar Fake Lag | `fakeLagEnabled` | Checkbox | Simula lag de rede | 602 |
| 32 | Quantidade (ms) | `fakeLagAmount` | Slider (10-500ms) | Duração do fake lag | 604 |
| 33 | Lag Adaptativo | `fakeLagAdaptive` | Checkbox | Ajusta lag automaticamente baseado em ping | 605 |
| 34 | Ativar Team-Check | `featureConfig.teamCheckEnabled` | Checkbox | Impede ataque em teammates | 613 |
| 35 | Método Team-Check | `featureConfig.teamCheckMethod` | Combo (0-15) | 16 métodos diferentes de verificação de time | 633 |

### 7.5 Tab BYPASS (linhas ~700-820)

| # | Feature | Variável | Tipo | Efeito | Linha |
|---|---------|----------|------|--------|-------|
| 36 | Bypass Manager | `bypassManager` | Sub-tab com 40 técnicas | Gerenciador de bypass com botões "Aplicar Todas" / "Reverter Todas" e botão individual por técnica | 739-820 |
| 37 | Bypass SDK | `drawBypassSDK()` | Sub-tab | Bypass do SDK de login anti-cheat (ffantihack) com opções: habilitar bypass, forçar login guest, spoof DeviceID, bloquear telemetria, logar todas as chamadas | 85-142 (Menu_append.cpp) |
| 38 | Bypass Network Monitor | `drawBypass()` | CollapsingHeader + sub-tabs | Monitor de requisições de rede com: Dashboard, Config, Requests Table, Details (Overview/Headers/Body/Raw/Response) | 2251-2291 |

### 7.6 Tab DUMPER (linhas ~822-886)

| # | Feature | Variável | Tipo | Efeito | Linha |
|---|---------|----------|------|--------|-------|
| 39 | Dump IL2Cpp Classes | `IL2CppDumper::DumpAll()` | Botão | Descarrega todas as classes IL2CPP para arquivo | 839-847 |
| 40 | Dump Classe Específica | `IL2CppDumper::DumpClass()` | Botão + Popup | Descarrega uma classe específica selecionada | 849-876 |

### 7.7 Tab NETWORK (linhas ~708-711)

| # | Feature | Efeito |
|---|---------|--------|
| 41 | Network Proxy (tab dedicada) | Captura pacotes TCP/UDP, decriptografia AES, exibição hex/ASCII, filtros TX/RX/plaintext, auto-scroll, botões start/stop/clear |

### 7.8 Features Ocultas / MenuExtras (MenuExtras.cpp)

> Estas features **não aparecem como abas visíveis** no Menu.cpp principal. São instanciadas via variáveis globais `ui*` e controladas por mecanismos externos ou outro fluxo de UI.

| # | Feature | Variável | Efeito |
|---|---------|----------|--------|
| 42 | Silent Aim (instância separada) | `uiSilentAim` / `silentAimFeature` | Silent aim como módulo independente (separado da instância do aimbot tab) |
| 43 | Aim Lock | `uiAimLock` / `aimLockFeature` | Lock-on contínuo no alvo |
| 44 | Aim Magnet | `uiAimMagnet` / `aimMagnetFeature` | Aimbot magnético (puxa alvo para a mira) |
| 45 | Magnet | `uiMagnet` / `magnetFeature` | Atração de inimigos para o jogador |
| 46 | Ghost Hack | `uiGhostHack` / `ghostHackFeature` | Modo fantasma (invisibilidade/movimento por paredes?) |
| 47 | Enemy Pull | `uiEnemyPull` / `enemyPullFeature` | Puxa inimigos para perto do jogador |
| 48 | Speed Run | `uiSpeedRun` | Hack de velocidade de movimento |
| 49 | Gold Body | `uiGoldBody` | Efeito visual de corpo dourado |
| 50 | Double Gun | `uiDoubleGun` | Dispara duas armas simultaneamente |
| 51 | Free Move | `uiFreeMove` | Movimento livre (no-clip) |
| 52 | Medi Run | `uiMediRun` | Corre enquanto usa medkit |
| 53 | No Recoil | `uiNoRecoil` | Sem recuo (via NikuHooks, alternativa à do aimbot tab) |
| 54 | Movement Shoot | `uiMovementShoot` | Atira enquanto se move |
| 55 | Up Player | `uiUpPlayer` | Eleva o jogador para cima (fly up) |
| 56 | Down Player | `uiDownPlayer` | Abaixa o jogador (fly down) |
| 57 | Teleport | `teleportFeature` | Teleporte para coordenada (acionado por botão externo) |
| 58 | Bypass Login SDK (auto-init) | `bypassSDKActive` | Inicialização automática do bypass do SDK de login com: enableBypass, forceGuestLogin, spoofDeviceID, blockTelemetry, logAllCalls |

### 7.9 Tabs de Debug (gated por `isDebugMode`)

| Tab | Conteúdo |
|-----|----------|
| Entity List | Lista filtrável de entidades (vivos, humanos, distância máxima, TeamID) |
| Entity Debug | Tabela completa com ponteiros, nomes, distância, TeamID, Alignment, PlayerID, Class, Status |
| Entity Props | Introspectão de campos Il2Cpp da entidade selecionada (Campo, Tipo, Offset, Valor) |
| Debug Player | Botão "Escanear Valores Possíveis" escaneia campos HP/Yaw/Pitch via Il2Cpp |
| Debug Aimbot | Dashboard com: nome do alvo, FOV/distância 3D, posição da câmera, rotação da câmera, rotação alvo, rotação calculada, log de erro |

---

## 8. Tabela Completa de Técnicas de Bypass

> Cobertura completa de todas as técnicas de bypass/anti-detecção encontradas **fora do Menu.cpp** (lógica de bypass pura).

### 8.1 Bypass Core (GhostSystems — compartilhado)

| ID | Técnica | Painel/Origem | Funções Hookadas / Alvo | Como Funciona | Arquivo |
|----|---------|--------------|------------------------|---------------|---------|
| BT-01 | **Filtro `/proc/self/maps` via memfd** | GhostSystems Core | `open()`, `openat()`, `fopen()` (libc) | Cria `memfd` com conteúdo de `/proc/self/maps` **filtrado** (remove linhas contendo `libghostsystems.so`). Hook intercepta leituras de `/proc/self/maps` e `/proc/self/smaps` e retorna o memfd filtrado. Biblioteca fica invisível. | `BypassLoginSDK.cpp:72-197` |
| BT-02 | **Syscall direto para memória** | GhostSystems Core | Nenhum hook — `syscall(__NR_process_vm_readv/writev)` diretamente | Escrita/leitura de memória cross-process via syscall kernel puro, ignorando hooks do libc. Usa `mincore()` para validar endereços. | `MemoryCloak.cpp:7-79` |
| BT-03 | **Motor de Inline Hook A64HookFunction** | GhostSystems Core (And64InlineHook) | Qualquer função ARM64 | Hook em nível de instrução (não PLT/GOT). Modifica prologo da função diretamente. Mais resistente a scans de PLT. | `And64InlineHook.cpp` |
| BT-04 | **Gerenciador de 40 Técnicas de Bypass (stub)** | GhostSystems Core (BypassManager) | Várias (ver abaixo) | Registro estruturado de 40 categorias de bypass. **Todas as implementações são stubs/log-only nesta versão**. Categorias: Code Injection (4), Memory Detection (4), Anti-Debug (4), Kernel-Level (4), Network (2), Emulators (4), HW Fingerprint (3), Updates (3), Analysis Tools (3), Behavior/ML (3), Obfuscation (3), Edge Cases (3). | `BypassManager.cpp:28-1045` |
| BT-05 | **Login SDK Stub Hooks** | GhostSystems Core | `LoginInit`, `VerifySession`, `GetDeviceID`, `CheckRoot`, `SendTelemetry`, `AntiCheatInit`, `ValidateToken`, `GetLoginType`, `IntegrityCheck` | Hooks planejados que retornam valores fake (ex: `GetDeviceID` spoofa ID, `CheckRoot` sempre retorna false, `ValidateToken` sempre true). **Stubs sem implementação real**. | `BypassLoginSDK.cpp:350-359` |
| BT-06 | **Patch Python `fix_bypass.py`** | Script de manutenção | `BypassLoginSDK.cpp` (regex replace) | Substitui automaticamente o stub de `Hook_SendTelemetry` para que ele repasse a chamada para a função original (forward) ao invés de bloquear. Reduz risco de detecção enquanto loga. | `fix_bypass.py:1-36` |

### 8.2 Bypass Específico por Painel

#### BRUXO / Hush (bypass.cpp)

| ID | Técnica | Funções Hookadas | Como Funciona | Arquivo |
|----|---------|-----------------|---------------|---------|
| BT-07 | **Interceptação de libc (ptrace/fopen/open/dlopen/getuid/readlink)** | `ptrace()`, `fopen()`, `open()`, `dlopen()`, `getuid()`, `readlink()` | Cada função verifica flag `bypass_enabled` (atomic bool, toggle via `enable_bypass()`). Quando habilitado: `ptrace` retorna -1, `fopen`/`open` retornam NULL/-1 para paths suspeitos, `dlopen` retorna NULL para libs de cheat/anti-cheat, `getuid` retorna 10000, `readlink` retorna -1 para `/proc/self/maps` e `/proc/self/exe`. | `BRUXO/Hush/bypass.cpp:37-138` |
| BT-08 | **JNI Anti-Debug** | `ApplicationInfo.isDebuggable()`, `com.garena.android.buildconfig.DEBUG`, `PackageManager.getPackageInfo()` | Retorna `false`/`nullptr` para métodos Java de detecção de debug. | `BRUXO/Hush/bypass.cpp:144-160` |
| BT-09 | **Offset Inline Hooks Agressivos (125+)** | 125+ offsets em 8 bibliotecas (ver abaixo) | Hooks genéricos (NOP) aplicados via `/proc/self/maps` para descobrir base addresses. Alvos: `libanogs.so` (5), `libil2cpp.so` (1), `libunity.so` (5), `libFirebaseCppApp-11_8_1.so` (3), `libcrashlytics-common.so` (1), `SH-Gpp2` (1), `libFFVoiceMagicVoiceEngine.so` (59), `libff_voice_engine.so` (39). | `BRUXO/Hush/bypass.cpp:164-330` |

**Bibliotecas e Offsets Específicos (BRUXO/Hush)**:
- `libanogs.so`: `0x21C`, `0x994`, `0x584f24`, `0x592f40`, `0x597074`
- `libil2cpp.so`: `0x7663b8`
- `libunity.so`: `0x8c6aa4`, `0x8c6aa8`, `0x8d9de4`, `0x8d9eac`, `0x8da12c`
- `libFirebaseCppApp-11_8_1.so`: `0x3ff874`, `0x400c2c`, `0x4015fc`
- `libcrashlytics-common.so`: `0x20dc0`
- `SH-Gpp2`: `0xa6d0`
- `libFFVoiceMagicVoiceEngine.so`: 59 offsets de `0x17c9b0` até `0x1da5a8`
- `libff_voice_engine.so`: 39 offsets de `0x255b00` até `0x2b1694`

#### GOAT CHEATS / gsmodz (bypass.cpp)

| ID | Técnica | Funções Hookadas | Como Funciona | Arquivo |
|----|---------|-----------------|---------------|---------|
| BT-10 | **Simplified libc + JNI Intercept Chain** | `ptrace`, `fopen`, `open`, `dlopen`, `getuid`, `readlink` + JNI `isDebuggable`/`DEBUG`/`getPackageInfo` | Mesmo padrão `dlsym(RTLD_NEXT)` do BRUXO mas com lista de bloqueio menor (sem nomes específicos de libs Garena). JNI retorna `false`/`nullptr` **sempre** (não condicional). | `GOAT CHEATS/gsmodz/bypass.cpp:26-85` |

#### GUJU MODS (zygisk.hpp + Main.cpp)

| ID | Técnica | Como Funciona | Arquivo |
|----|---------|---------------|---------|
| BT-11 | **DLCLOSE self-unload** | `preAppSpecialize` define `Option::DLCLOSE_MODULE_LIBRARY`. Após especialização Zygisk, a biblioteca do módulo é `dlclose`-ada. Os trampolines de hook (escritos antes do unload) permanecem na memória. Biblioteca desaparece do `/proc/self/maps`. | `GUJU MODS/zygisk.hpp:24-52` |
| BT-12 | **FORCE_DENYLIST_UNMOUNT** | Expõe `Option::FORCE_DENYLIST_UNMOUNT = 0` que força desmonte da denylist do Magisk no namespace do processo. Remove arquivos Magisk/module da visão do processo. | `GUJU MODS/zygisk.hpp:167-169` |
| BT-13 | **PLT Hook System nativo do Zygisk** | `Api::pltHookRegister()` + `pltHookCommit()` — hook por device+inode ELF matching (mais seguro que `dlsym`). | `GUJU MODS/zygisk.hpp:246-261` |
| BT-14 | **fake_dlfcn** | No Android SDK >= 24, substitui `dlopen`/`dlsym` com implementação própria que parseia ELF diretamente (via `/proc/self/maps` + mmap + leitura de `.dynsym`/`.dynstr`). Bypassa hooks de anti-cheat em `dlopen`/`dlsym`. SDK < 24 usa funções reais. | `GUJU MODS/*/fake_dlfcn.cpp:81-312` |
| BT-15 | **Offsets Randomizados** | `getRealOffset()` adiciona `rand() % 0x1000` ao endereço base. Usa `dlopen(RTLD_NOLOAD)` primeiro, cai para parse de `/proc/self/maps`, depois dlopen sem NOLOAD, depois raw offset. Seed: `time(NULL) ^ getpid()`. | `GUJU MODS/hide.h:136-207` |
| BT-16 | **Hook JNI via Zygisk API** | `Api::hookJniNativeMethods()` hooka métodos nativos JNI por nome/assintura, salvando ponteiros originais. | `GUJU MODS/zygisk.hpp:238-244` |
| BT-17 | **Patches Hex AArch64** | `patchLib()` injeta bytes diretamente em offsets de libs. Macros pré-definidas: FALSE (`mov x0, #0; ret` → `00 00 80 D2 C0 03 5F D6`) e TRUE (`mov x0, #1; ret` → `20 00 80 D2 C0 03 5F D6`). | `GUJU MODS/Main.cpp:96-103` |
| BT-18 | **Emulator Detection Bypass** | Hook em `COW.GameFacade.IsEmulator()` retorna `false` (via `_PCcheck()`). Também seta `IsEverythingLoad = true` para pular verificações de load. | `GUJU MODS/Main.cpp:161-166, 183` |
| BT-19 | **Screen/Resolution Hook (comentado)** | Hook em `SceneGraphicsQuality.SetGraphicsQuality()` e `Screen.SetResolution()` para impedir que anti-cheat mude resolução e detecte overlay/ESP. | `GUJU MODS/Main.cpp:227-228` (comentado) |

#### AXL / AxlMods (Hook.h + cheat.cpp)

| ID | Técnica | Como Funciona | Arquivo |
|----|---------|---------------|---------|
| BT-20 | **Obfuscação OBFUSCATE()** | Strings sensíveis (nomes de libs, classes, métodos, paths) envolvidas em macro `OBFUSCATE("...")`. Dificulta análise estática de strings. | `AXL/AxlMods/Hook.h` (ex: linha 126, 1088) |
| BT-21 | **Reset Guest com Deleção de Arquivo** | Quando flag `Reset` está ativa, hook `ResetGuest()` deleta `/storage/emulated/0/com.garena.msdk/guest100067.dat` para resetar identidade de conta guest banida. | `AXL/AxlMods/Hook.h:1083-1092` |
| BT-22 | **Ghost Network Message Suppression** | Hook em `NeedSendMessage` retorna `true` (suprime envio) quando feature Ghost está ativa. Esconde ações do jogador no servidor. | `AXL/AxlMods/Hook.h:1063-1072` |

#### SYZEN (cheat.cpp)

| ID | Técnica | Como Funciona | Arquivo |
|----|---------|---------------|---------|
| BT-23 | **Delay de Carregamento de Biblioteca** | Espera em loop `while (!getLibraryLoaded("libil2cpp.so")) sleep(3)` antes de aplicar hooks. Evita detecção baseada em ordem de carregamento de bibliotecas. | `SYZEN/cheat.cpp:128-131` |
| BT-24 | **Hook Padrão eglSwapBuffers** | Hook em `eglSwapBuffers` via `A64HookFunction` para renderizar overlay ImGui a cada frame. Padrão compartilhado entre SYZEN, BRUXO e GOAT. | `SYZEN/cheat.cpp:128-149` |

---

## 9. Módulos Compartilhados Independentes de Painel

> Estes módulos não pertencem a nenhum painel específico. São features independentes que podem ser incluídas em qualquer painel via `#include`.

| Módulo | Caminho | Função | Como Usar |
|--------|---------|--------|-----------|
| **AimLock** | `Features/AimLock.h` / `.cpp` | Mira sempre travada no alvo mais próximo (lock-on contínuo). | Incluir header, chamar `aimLockFeature.Enable(bool)` no código do painel. |
| **AimMagnet** | `Features/AimMagnet.h` / `.cpp` | Aimbot com atração magnética (puxa alvo para mira). | Incluir header, toggle `aimMagnetFeature`. |
| **EnemyPull** | `Features/EnemyPull.h` / `.cpp` | Puxa inimigos para perto do jogador (teleporta para perto). | Incluir header, toggle `enemyPullFeature`. |
| **GhostHack** | `Features/GhostHack.h` / `.cpp` | Modo fantasma (invisibilidade / noclip parcial?) | Incluir header, toggle `ghostHackFeature`. |
| **Magnet** | `Features/Magnet.h` / `.cpp` | Atração gravitacional genérica de entidades. | Incluir header, toggle `magnetFeature`. |
| **NoRecoil** | `Features/NoRecoil.h` / `.cpp` | Remove recuo da arma zerando parâmetros de recoil a cada tiro. | Incluir header, toggle `nikuHooksFeature` (via NikuHooks). |
| **Teleport** | `Features/Teleport.h` / `.cpp` | Teleporta jogador para coordenadas específicas. | Incluir header, chamar método de teleporte. |
| **UpDown** | `Features/UpDown.h` / `.cpp` | Eleva (`UpPlayer`) ou abaixa (`DownPlayer`) o jogador. | Incluir header, toggle `upDownFeature`. |
| **FakeLag** | `Features/FakeLag.h` / `.cpp` | Simula lag de rede atrasando pacotes. | Incluir header, toggle `fakeLagEnabled`. |
| **NikuHooks** | `Features/NikuHooks.h` / `.cpp` | Coleção de hooks secundários: SpeedRun, GoldBody, DoubleGun, FreeMove, MediRun, NoRecoil, MovementShoot. | Incluir header, toggle flags correspondentes. |
| **ChamsManager / ChamsGlHook** | `ChamsManager.cpp` / `ChamsGlHook.cpp` | Hook do OpenGL para alterar shaders de renderização de jogadores (chams por paredes). | Incluir, chamar no hook OpenGL. |
| **BypassManager** | `BypassManager.h` / `.cpp` | Gerenciador centralizado de 40 técnicas de bypass (stubs/log atualmente). | Incluir, instanciar `bypassManager`, chamar `ApplyAll()` / `RevertAll()`. |
| **BypassLoginSDK** | `BypassLoginSDK.h` / `.cpp` | Stub de hooks para o SDK de login da Garena (spoof DeviceID, bypass root check, bloqueio de telemetria). | Incluir, inicializar no startup do módulo. |
| **MemoryCloak** | `MemoryCloak.h` / `.cpp` | Cloak de arquivos em memória (filtra `/proc/self/maps` via memfd) + syscall direto para memória. | Incluir, chamar no construtor do módulo. |
| **SharedAccess** | `Features/SharedAccess.h` | Interface para compartilhar dados comuns (chaves AES, session IDs) entre painéis. | Incluir onde necessário. |
| **IFeature** | `Features/IFeature.h` | Classe base abstrata para módulos de feature (padroniza API Enable/Disable/Update). | Herdar para novas features. |

---

## 10. Guia de Reutilização — Como Implementar Features Existentes no Seu Painel

### Passo 1: Identifique a Feature no Menu.cpp

- Abra `Menu.cpp` e procure pelo **UI Label** da feature (ex: "Silent Aim", "Enemy Pull").
- Anote a **variável** associada (ex: `aimbotSilentAim`, `enemyPullFeature`).
- Verifique a **linha** onde a UI é renderizada.

### Passo 2: Encontre o Arquivo de Implementação

- A maioria das features vive em `Features/*.h` e `Features/*.cpp` (arquivos independentes de painel).
- Para features específicas de um painel, procure na pasta do painel correspondente:
  - AXL → `Features/SRCS_PAINEIS/AXL/AxlMods/`
  - BRUXO → `Features/SRCS_PAINEIS/BRUXO/Hush/` ou `ZygModule/`
  - SYZEN → `Features/SRCS_PAINEIS/SYZEN/ZygModule/`
  - GOAT → `Features/SRCS_PAINEIS/GOAT CHEATS/zygisk/` ou `gsmodz/`
- Para bypass, consulte a [Tabela de Bypass](#8-tabela-completa-de-técnicas-de-bypass).

### Passo 3: Adicione o Header no Seu Código

```cpp
#include "Features/FeatureName.h"
```

### Passo 4: Inicialize a Feature no Startup do Módulo

No seu `cheat.cpp` ou entry point do Zygisk, após o carregamento do Il2Cpp:

```cpp
nomeDaFeature.Enable(true);  // ou chamada de Init específica
```

### Passo 5: Exponha no ImGui

No loop de renderização do seu painel (dentro do bloco `ImGui::BeginTabItem("SEU_PAINEL")`):

```cpp
ImGui::Checkbox("Nome da Feature", &nomeDaVariavelGlobal);
```

### Passo 6: Recompile

- Adicione os novos `.cpp` no `compile.bat` se criar arquivos novos.
- Bibliotecas compartilhadas (`KittyMemory/`, `Misc/imgui/`, `xdl/`) **não precisam** ser readicionadas se já estiverem no `compile.bat`.

### Passo 7: Teste no Device

- Use `adb logcat` com TAGs apropriados (ex: `adb logcat | grep "GhostSystems"`) para debug.
- Habilite `isDebugMode` no código se precisar ver Entity List e Debug Aimbot.

---

## Observações Finais

- **Menu.cpp** é o arquivo canônico da UI. Qualquer feature listada na Seção 7 está acessível lá.
- **Bypasses** estão espalhados por múltiplos painéis e arquivos. A Seção 8 lista todos com seus respectivos arquivos de origem.
- **GUJU MODS** é o painel mais "experimental" e contém técnicas de injeção mais stealth (DLCLOSE, fake_dlfcn, PLT hooks nativos).
- **BRUXO/Hush** é o mais agressivo em termos de anti-cheat (125+ inline hooks).
- **GOAT CHEATS/gsmodz** é uma versão simplificada do BRUXO.
- **SYZEN** inclui um sistema de login/licença completo em `eLogin/`.
- **AXL** tem o conjunto mais diversificado de features de jogo (ESP, aimbot, speedhack, magnet, teleporte, etc.) e usa obfuscação de strings em massa.

---

*Documento gerado para mapeamento de sources e reutilização futura de features. Última atualização: análise baseada em `Menu.cpp` canônico + bypass files de todos os painéis.*
