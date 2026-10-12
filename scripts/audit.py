#!/usr/bin/env python3
"""Offline checks for delivery completeness, vendor integrity and project consistency."""
from pathlib import Path
import hashlib
import json
import re
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parent.parent


def require(condition, message):
    if not condition:
        raise SystemExit("AUDIT FAILED: " + message)


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


required = [
    "README.md", "LICENSE", "THIRD_PARTY.md", "Makefile", ".gitignore",
    "scripts/check_elf.py", "scripts/flash.py", "scripts/install_toolchain_macos.sh",
    "scripts/verify.sh", "tests/test_main.c", "tests/test_gpio_model.c",
    "simulator/gpio_model.c", "simulator/gpio_model.h", "simulator/morse_demo.c",
]
required += ["docs/" + name + ".md" for name in (
    "INSTALLATION", "HARDWARE_SETUP", "BUILD_AND_FLASH", "USER_GUIDE",
    "TROUBLESHOOTING", "ARCHITECTURE", "DEFENSE_PREPARATION", "GPIO_SIMULATOR",
    "HARDWARE_VALIDATION", "VERIFICATION", "RESEARCH", "IMPLEMENTATION_PLAN")]
for path in required:
    require((ROOT / path).is_file(), "Missing " + path)

manifest = json.loads(read("scripts/vendor_manifest.json"))
for item in manifest["files"]:
    path = ROOT / item["path"]
    require(path.is_file(), "Missing vendor file " + item["path"])
    require(hashlib.sha256(path.read_bytes()).hexdigest() == item["sha256"],
            "Changed vendor file " + item["path"])

for mcu, directory, part in (("F407", "firmware", "STM32F407VGT"),
                              ("F427", "firmware-f427", "STM32F427VIT")):
    folder = ROOT / directory
    name = "Lab1_" + mcu
    project = ET.parse(folder / ".project").getroot()
    require(project.findtext("name") == name, "Project name " + directory)
    links = {}
    for link in project.findall("./linkedResources/link"):
        uri = link.findtext("locationURI", "")
        require(uri.startswith("PARENT-1-PROJECT_LOC/"), "Nonrelative Eclipse link")
        path = folder.parent / uri.split("/", 1)[1]
        require(path.is_dir(), "Broken Eclipse link " + str(path))
        links[link.findtext("name")] = path
    cproject = ET.parse(folder / ".cproject").getroot()
    configurations = cproject.findall(".//configuration")
    require({c.get("name") for c in configurations} == {"Debug", "Release"},
            "Missing build configurations " + name)
    for config in configurations:
        options = config.findall(".//option")
        require(any(o.get("superClass", "").endswith(".target_mcu") and
                    o.get("value") == part + "x" for o in options), "Wrong MCU " + name)
        defines = [v.get("value") for o in options
                   if o.get("valueType") == "definedSymbols"
                   for v in o.findall("listOptionValue")]
        require("STM32" + mcu + "xx" in defines and "USE_HAL_DRIVER" in defines,
                "Device defines " + name)
        for option in options:
            if option.get("valueType") == "includePath":
                for entry in option.findall("listOptionValue"):
                    # CDT's relative compiler paths start in Debug/Release.
                    path = folder / config.get("name") / entry.get("value")
                    require(path.resolve().is_dir(), "Broken include path " + str(path))
        sources = {x.get("name") for x in config.findall("./sourceEntries/entry")}
        require(sources == {"Core", "Drivers", "Startup"}, "Source roots " + name)
        for source in sources:
            require(links.get(source, folder / source).is_dir(), "Missing source " + source)
        require(any(o.get("superClass", "").endswith(".c.linker.option.script") and
                    part + "X_FLASH.ld" in o.get("value", "") for o in options),
                "Wrong linker selection " + name)
    require((folder / (part + "X_FLASH.ld")).is_file(), "Missing linker " + name)
    require((folder / "Startup" / ("startup_stm32" + mcu.lower() + "xx.s")).is_file(),
            "Missing startup " + name)
    ioc = dict(line.split("=", 1) for line in read(directory + "/" + name + ".ioc").splitlines()
               if "=" in line and not line.startswith("#"))
    require(ioc["Mcu.UserName"] == part + "x" and ioc["Mcu.CPN"] == part + "6",
            "Wrong IOC part " + name)
    require(ioc["PC15-OSC32_OUT.Signal"] == "GPIO_Input", "Button IOC")
    require(ioc["PC15-OSC32_OUT.GPIO_PuPd"] == "GPIO_NOPULL", "Button pull configuration")
    for pin in (13, 14, 15):
        require(ioc[f"PD{pin}.Signal"] == "GPIO_Output", "LED IOC")
    for pin, signal in (("PA13", "SYS_JTMS-SWDIO"), ("PA14", "SYS_JTCK-SWCLK"),
                        ("PA15", "SYS_JTDI"), ("PB3", "SYS_JTDO-SWO"), ("PB4", "SYS_JTRST")):
        require(ioc.get(pin + ".Signal") == signal, "JTAG IOC " + pin)
    require(ioc["RCC.SYSCLKFreq_VALUE"] == "16000000", "Clock IOC")
    require(ioc["NVIC.SysTick_IRQn"].split("\\:")[1] == "15", "SysTick priority IOC")
    require(ioc["ProjectManager.FirmwarePackage"] == "STM32Cube FW_F4 V1.27.1", "HAL version")
    require(ioc["ProjectManager.KeepUserCode"] == "true", "User-code protection")
    launch = ET.parse(folder / (name + " FTDI Debug.launch")).getroot()
    attributes = {x.get("key"): x.get("value") for x in launch}
    require(attributes["org.eclipse.cdt.launch.PROJECT_ATTR"] == name, "Launch project")
    require(attributes["com.st.stm32cube.ide.mcu.debug.openocd.DBG_INTERFACE"] == "Jtag",
            "Launch transport")
    script = attributes["com.st.stm32cube.ide.mcu.debug.openocd.OPENOCD_SCRIPT"]
    script = script.replace("${workspace_loc:/" + name + "}", str(folder))
    require(Path(script).is_file(), "Launch script missing")
    require(part + "x" in Path(script).read_text(), "Debugger target name")

# This checks source organization; functional tests check behavior separately.
def without_comments(source):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)


core = ROOT / "firmware/Core"
for file in (core / "Src").glob("*.c"):
    if file.name == "system_stm32f4xx.c":
        continue  # Unchanged CMSIS vendor file, checked by hash above.
    source = without_comments(file.read_text())
    require(not re.search(r"\bHAL_Delay\s*\(", source), "Blocking delay " + file.name)
    if file.stem in ("button", "led", "morse"):
        require(not re.search(r"\b(?:for|while)\s*\(", source), "Loop in runtime driver/app " + file.name)
    if file.stem == "morse":
        require(not re.search(r"HAL_GPIO|GPIO[A-K]|GPIO_PIN_|#include\s+\"main.h\"", source),
                "Board detail in Morse application")
gpio = read("firmware/Core/Src/gpio.c")
for required_text in ("__HAL_RCC_GPIOC_CLK_ENABLE", "__HAL_RCC_GPIOD_CLK_ENABLE",
                      "GPIO_NOPULL", "GPIO_MODE_INPUT", "GPIO_MODE_OUTPUT_PP"):
    require(required_text in gpio, "Missing GPIO initialization " + required_text)
require(re.search(r"#define\s+TICK_INT_PRIORITY\s+15U", read("firmware/Core/Inc/stm32f4xx_hal_conf.h")),
        "SysTick priority must match the IOC")

# No developer-specific machine paths in build metadata/configuration/scripts.
files = list((ROOT / "scripts").glob("*.py")) + list((ROOT / "scripts").glob("*.sh"))
files += [ROOT / "Makefile"]
for directory in ("firmware", "firmware-f427", "debugger"):
    folder = ROOT / directory
    files += [p for p in folder.rglob("*") if p.is_file() and
              (p.name in (".project", ".cproject") or p.suffix in (".ioc", ".launch", ".cfg"))]
for file in files:
    require(not re.search(r"/(?:Users|home)/[^/\s]+/|[A-Z]:\\\\", file.read_text()),
            "Machine-specific path " + str(file.relative_to(ROOT)))

markdown = [ROOT / "README.md", ROOT / "THIRD_PARTY.md"] + list((ROOT / "docs").glob("*.md"))
for file in markdown:
    for target in re.findall(r"\[[^\]\n]+\]\(([^)\n]+)\)", file.read_text()):
        if target.startswith(("http://", "https://", "#", "mailto:")):
            continue
        path = target.split("#", 1)[0].strip("<>")
        require((file.parent / path).exists(), "Broken link " + target + " in " + file.name)
print(f"Audit passed: {len(manifest['files'])} vendor hashes, both project configurations, "
      "relative paths, runtime source checks and documentation links")
