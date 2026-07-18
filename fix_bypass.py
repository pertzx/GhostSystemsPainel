import re

with open('BypassLoginSDK.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

safe_hook = """void BypassLoginSDK::Hook_SendTelemetry(void* instance, void* data) {
    auto& sdk = Instance();
    
    if (!Instance().config.enableBypass) {
        if (sdk.origSendTelemetry) {
            auto orig = reinterpret_cast<void(*)(void*, void*)>(sdk.origSendTelemetry);
            orig(instance, data);
        }
        return;
    }
    
    AddBypassLog(\"SendTelemetry\", \"PASSANDO\",
                 \"Telemetria logada (safe)\",
                 ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
    
    if (sdk.origSendTelemetry) {
        auto orig = reinterpret_cast<void(*)(void*, void*)>(sdk.origSendTelemetry);
        orig(instance, data);
    }
}"""

# Find and replace the function
pattern = r'(void BypassLoginSDK::Hook_SendTelemetry\(void\* instance, void\* data\).*?)(// Hook 6)'
replacement = safe_hook + '\n\n// Hook 6'

new_content = re.sub(pattern, replacement, content, flags=re.DOTALL)

with open('BypassLoginSDK.cpp', 'w', encoding='utf-8') as f:
    f.write(new_content)

print(\"Done\")
