# GhostSystems - BypassLoginSDK v1.0

> **AVISO**: Este documento e dedicado ao desenvolvimento de seguranca e pesquisa. O uso indevido em jogos online pode viol的定位 so direitos dos fabricantes. Use por sua conta e risco.

## Sumario

- [1. O que e o BypassLoginSDK](#1-o-que-e-o-bypassloginsdk)
- [2. Estrutura dos Arquivos](#2-estrutura-dos-arquivos)
- [3. Como Funciona](#3-como-funciona)
- [4. Como Usar no Seu Projeto](#4-como-usar-no-seu-projeto)
- [5. Sistema de Logs](#5-sistema-de-logs)
- [6. Como Atualizar Apos Patch (Muito Importante)](#6-como-atualizar-apos-patch-muito-importante)
- [7. Troubleshooting](#7-troubleshooting)

---

## 1. O que e o BypassLoginSDK

O `BypassLoginSDK` e um modulo **independente e dedicado** para interceptar (hook) e modificar as funcoes do SDK de login do Free Fire. Ele trabalha em paralelo com o `BypassManager` existente, mas nao interfere nele.

### Diferenca entre BypassManager e BypassLoginSDK

| Feature | `BypassManager` | `BypassLoginSDK` |
|---------|-----------------|-------------------|
| Foco | Anti-cheat generico (40 tecnicas) | SDK de login especifico |
| Hooks | Passivos/tecnicas de ocultacao | Hooks diretos em funcoes do SDK |
| Log | Basico | Detalhado com funcao/linha |
| Atualizacao | Raramente (tecnicas genericas) | Freq. (RVAs mudam a cada patch) |

---

## 2. Estrutura dos Arquivos

```
GhostSystemsPainel/
  ├── BypassLoginSDK.h      <-- Header com declaracoes e config
  ├── BypassLoginSDK.cpp    <-- Implementacao completa com logs
  └── (novos arquivos)
```

---

## 3. Como Funciona

O módulo instala **8 hooks** em funções críticas do namespace `ffantihack` (anti-cheat do Free Fire):

| # | Hook | Método Original | Classe | RVA | Retorno Padrão |
|---|------|----------------|--------|-----|----------------|
| 1 | `FFAntiCheat_Init` | `FJHAGCJDKPN()` | `DCKLGOGDPCH` | `0x3e8354c` | Neutralizado |
| 2 | `VerifySession` | `NDECONJAANP(String)` | `PENGBDFHIIN` | `0x3e87f80` | Sempre sucesso |
| 3 | `GetDeviceID` | `DJELBEFGCAK()` | `DCKLGOGDPCH` | `0x3e83e20` | DeviceID spoofado |
| 4 | `CheckRoot` | `JKGFBIEOBDF()` | `DCKLGOGDPCH` | `0x3e83674` | Sempre "não-root" |
| 5 | `SendTelemetry` | `LOEBJODHEPP()` | `DCKLGOGDPCH` | `0x3e83da0` | Bloqueado |
| 6 | `AntiCheat_Integrity` | `KCCPOHCKDPK()` | `DCKLGOGDPCH` | `0x3e84c50` | Neutralizado |
| 7 | `ValidateToken` | `NDECONJAANP(String)` | `PENGBDFHIIN` | `0x3e87f80` | Sempre válido |
| 8 | `GetLoginType` | `GKCOOPMPOAD()` | `DCKLGOGDPCH` | `0x3e84cdc` | Status normal |

### Processo de Hook

```
[Funcao Original] ----(jump)----> [Funcao Hook] ----> [Retorno Modificado]
        |                              |
        |                            Logs cada
        |                            chamada
        v
[Trampoline] ---- (se necessario)
```

---

## 4. Como Usar no Seu Projeto

### 4.1 Incluir o Header

```cpp
#include "BypassLoginSDK.h"
```

### 4.2 Inicializar (chamar uma vez, preferencialmente no `JNI_OnLoad`)

```cpp
using namespace GhostSystems;

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    // Configura o bypass
    LoginSDKConfig config;
    config.enableBypass     = true;   // Ativa o bypass
    config.forceGuestLogin  = true;   // Forca login como Guest
    config.spoofDeviceID    = true;   // Gera DeviceID falso
    config.blockTelemetry   = true;   // Bloqueia telemetria
    config.logAllCalls      = true;   // Loga todas as chamadas
    
    // Inicializa o bypass
    if (BypassLoginSDK::Instance().Initialize(config)) {
        LOGI("BypassLoginSDK iniciado com sucesso!");
    } else {
        LOGE("Falha ao iniciar BypassLoginSDK!");
    }
    
    return JNI_VERSION_1_6;
}
```

### 4.3 Checar Status

```cpp
if (BypassLoginSDK::Instance().HasAnyHookApplied()) {
    LOGI("Hooks de login ativos!");
}
```

### 4.4 Obter Informacoes dos Hooks (para UI)

```cpp
for (const auto& hook : BypassLoginSDK::Instance().GetHookList()) {
    LOGI("Hook: %s | Status: %s | Desc: %s",
        hook.name.c_str(),
        hook.isApplied ? "ATIVO" : "INATIVO",
        hook.description.c_str());
}
```

---

## 5. Sistema de Logs

Todos os hooks possuem log detalhado com a seguinte formatacao:

```
[OK]  [Funcao:LINHA] [Hook_NomeDoHook] ENTER - parametros
[OK]  [Funcao:LINHA] [Hook_NomeDoHook] RETURN - resultado (motivo)
[ERR] [Funcao:LINHA] [Hook_NomeDoHook] ERRO - detalhes
```

### Filtrar Logs no Android Studio / logcat:

```bash
# Filtrar apenas logs do bypass
adb logcat -s GhostSDK_Bypass:I

# Ver tudo inclusive debug (se logAllCalls=true)
adb logcat -s GhostSDK_Bypass:D
```

---

## 6. Como Atualizar Apos Patch (Muito Importante!!!)

> **ATENCAO**: Os offsets (RVA) mudam em quase toda atualizacao do jogo. ESTE E O PASSO MAIS IMPORTANTE.

### 6.1 Passo a Passo

#### Passo 1: Dump do jogo atualizado

1. Use o `Il2CppDumper` para extrair o dump do novo APK
2. Copie o novo `script.json` ou `dump.cs` para o projeto

#### Passo 2: Encontrar os Novos RVAs

Abra o `dump.cs` e busque pelas funcoes de login. Use os termos:
- `LoginInit`
- `VerifySession`
- `GetDeviceID`
- `CheckRoot`
- `SendTelemetry`
- `AntiCheatInit`
- `ValidateToken`
- `GetLoginType`

Exemplo de busca no dump.cs:
```
public Void LoginInit(LoginType type) { }
// RVA: 0x15F4A80
```

#### Passo 3: Atualizar o `BypassLoginSDK.cpp`

Edite as linhas onde os RVAs sao definidos:

```cpp
// Hook 1: LoginInit
if (SetupHook("LoginInit", "...",
    reinterpret_cast<void*>(0xNOVO_RVA_AQUI),  // <-- ATUALIZAR!
    reinterpret_cast<void*>(Hook_LoginInit),
    &origLoginInit, true)) {
```

#### Passo 4: Compilar e Testar

```bash
# Exemplo de compilacao via NDK
ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk
```

### 6.2 Template de Update (copiar para cada patch)

Segue abaixo o template completo com os locais para atualizar:

#### NO `BypassLoginSDK.cpp`:

Busque por estas linhas (procure por "// Hook" ou "RVA:"):

```cpp
// H1: FJHAGCJDKPN  (Anti-cheat init)        -> RVA: 0x3e8354c
// H2: NDECONJAANP  (Valida sessao)           -> RVA: 0x3e87f80
// H3: DJELBEFGCAK  (Device fingerprint)      -> RVA: 0x3e83e20
// H4: JKGFBIEOBDF  (Verificacao periodica)   -> RVA: 0x3e83674
// H5: LOEBJODHEPP  (Envia report)            -> RVA: 0x3e83da0
// H6: KCCPOHCKDPK  (Integridade APK)         -> RVA: 0x3e84c50
// H7: NDECONJAANP  (Valida sessao - repete)   -> RVA: 0x3e87f80
// H8: GKCOOPMPOAD  (Status anti-cheat)       -> RVA: 0x3e84cdc
```

**IMPORTANTE: Atualize NOME do metodo e o RVA no comentario do codigo:**

```cpp
// Hook 1: FJHAGCJDKPN (DCKLGOGDPCH.FJHAGCJDKPN) - RVA: 0x3e8354c <- ATUALIZE AQUI
if (SetupHook("...", "...", reinterpret_cast<void*>(0xNOVO_RVA), ...))
```

### 6.3 Verificacao Pos-Update

Apos atualizar os RVAs, verifique no logcat:

```
Esperado:
[ApplyHooks] Iniciando aplicacao de 8 hooks...
[ApplyHooks] === Hook 1/8: LoginInit ===
[SetupHook] Hook 'LoginInit' configurado com sucesso  <-- IMPORTANTE
[ApplyHooks] LoginInit -> HOOK APLICADO               <-- IMPORTANTE
[ApplyHooks] ==============================================
[ApplyHooks] RESUMO:
[ApplyHooks]   Total aplicados: 8/8                   <-- DEVE SER 8/8
[ApplyHooks]   Criticos aplicados: 8                  <-- DEVE SER 8
[ApplyHooks]   Falhas: 0                              <-- DEVE SER 0
[ApplyHooks]   Status: TUDO OK                        <-- DEVE MOSTRAR ISSO
```

**Se aparecer "FALHA" em algum hook:**

1. Verifique se o RVA esta correto no dump
2. Verifique se o modulo e o mesmo (libil2cpp.so vs libunity.so)
3. Verifique se nao houve mudanca nos parametros da funcao

---

## 7. Troubleshooting

### Problema: `Falha ao obter handle de libil2cpp.so`

**Causa**: O modulo nao encontrou o jogo na memoria.
**Solucao**: Verifique se o nome do modulo em `TARGET_MODULE` no `.cpp` esta correto.

### Problema: `Hook aplicado, mas jogo crasha`

**Causa**: Os parametros da funcao mudaram na nova versao do jogo.
**Solucao**: Verifique o `dump.cs` e compare a assinatura da funcao:

```
// Versao antiga:
public Void LoginInit(LoginType type) { }

// Versao nova pode ser:
public Void LoginInit(LoginType type, Boolean flag) { }
```
Se mudou, atualize o hook correspondente na implementacao.

### Problema: `Aplicado: 0/8, Falhas: 8`

**Causa**: Todos os RVAs estao incorretos (versao completamente diferente).
**Solucao**: Faca o dump novamente e atualize TODOS os RVAs.

### Problema: `Nenhum log aparece no logcat`

**Causa**: Tag do logcat esta errada ou log foi filtrado.
**Solucao**: Use exatamente:
```bash
adb logcat -s GhostSDK_Bypass:D
```

### Problema: `Bypass funciona, mas ainda toma ban`

**Causa**: O anti-cheat pode ter deteccao adicional nao coberta por este modulo.
**Solucao**:
1. Verifique se o `BypassManager` (as 40 tecnicas) tambem esta ativo
2. Considere que o ban pode ser pelo comportamento (mire muito humanizado)
3. Verifique se o DeviceID spoofado ja foi banido anteriormente

---

## Historico de Versoes

| Versao | Data | Notas |
|--------|------|-------|
| v1.0 | 2026-06-30 | Versao inicial com 8 hooks e documentacao completa |

---

> **Nota Final**: Sempre mantenha este arquivo `BYPASS_LOGIN_SDK.md` atualizado junto com os RVAs no codigo. Documente cada patch com a versao do jogo para referencia futura.
