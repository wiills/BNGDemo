#!/usr/bin/env python3
"""BlueprintNodeGraph refactoring: move files + rename classes + update includes."""

import os
import re
import shutil
from pathlib import Path

PLUGIN_ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = PLUGIN_ROOT / "Source"

# old basename -> new path relative to BlueprintTool/
FILE_MAP_RUNTIME = {
    # Common
    "ExLatentProxyDefine.h": "Common/ExLatentProxyDefine.h",
    "ExWaitBranchCompletionMode.h": "Common/ExBranchMode.h",
    "ExBlueprintDebugBubble.h": "Common/ExBlueprintDebugBubble.h",
    "ExK2NodeTimeoutLatentAction.h": "Common/ExLatentTimeoutAction.h",
    "ExWaitAction.h": "Common/ExWaitAction.h",
    "ExSubsystemGetter.h": "Common/ExSubsystemGetter.h",
    "ExSaveGameTypes.h": "Common/ExSaveGameTypes.h",
    # Subsystems
    "ExBlueprintNodeGraphDebugSubsystem.h": "Subsystems/ExBlueprintNodeGraphDebugSubsystem.h",
    "ExWorldPartitionSubsystem.h": "Subsystems/ExWorldPartitionSubsystem.h",
    # AsyncActions
    "ExAsyncActionBase.h": "AsyncActions/ExBase_AsyncAction.h",
    "ExAsyncLoadAsset.h": "AsyncActions/ExAsyncAction_LoadAsset.h",
    "ExAsyncStreamLevel.h": "AsyncActions/ExAsyncAction_StreamLevel.h",
    "ExLatentSpawnProxy.h": "AsyncActions/ExAsyncAction_SpawnActor.h",
    "ExGameplayTagProxy.h": "AsyncActions/ExAsyncAction_GameplayTag.h",
    "ExNetworkProxy.h": "AsyncActions/ExAsyncAction_Network.h",
    # Proxies
    "ExWaitConditionProxy.h": "Proxies/ExProxy_WaitCondition.h",
    "ExWaitBranchProxy.h": "Proxies/ExProxy_WaitBranch.h",
    "ExAsyncBlendPercent.h": "Proxies/ExProxy_BlendPercent.h",
    "ExLoopDelayProxy.h": "Proxies/ExProxy_LoopDelay.h",
    "ExForLoopWithDelayProxy.h": "Proxies/ExProxy_ForLoopWithDelay.h",
    "ExLatentTaskProxy.h": "LatentTasks/ExLatentTask_Custom.h",
    # LatentTasks
    "ExLatentTaskBase.h": "LatentTasks/ExBase_LatentTask.h",
    "ExLatentTaskInterface.h": "LatentTasks/ExLatentTaskInterface.h",
    "ExLatentTaskForAttach.h": "LatentTasks/ExLatentTask_ForAttach.h",
    "ExSaveableLatentTask.h": "LatentTasks/ExLatentTask_Saveable.h",
    # Libraries
    "ExBlueprintNodeLibrary.h": "Libraries/ExBlueprintNodeLibrary.h",
    "ExSaveGameLibrary.h": "Libraries/ExSaveGameLibrary.h",
    # Assets
    "ExGraphAsset.h": "Assets/ExGraphAsset.h",
    "ExLevelTrigger.h": "Assets/ExLevelTrigger.h",
}

FILE_MAP_RUNTIME_CPP = {k.replace(".h", ".cpp"): v.replace(".h", ".cpp") for k, v in FILE_MAP_RUNTIME.items() if k != "ExLatentTaskInterface.h"}

# ExLatentActionManager handled separately in split_latent_action_manager()
FILE_MAP_RUNTIME.pop("ExLatentActionManager.h", None)

FILE_MAP_EDITOR = {
    "ExK2Node_AsyncBase.h": "K2Nodes/ExK2Node_AsyncBase.h",
    "ExK2Node_ShowBase.h": "K2Nodes/ExK2Node_ShowBase.h",
    "ExK2Node_SwitchValue.h": "K2Nodes/ExK2Node_SwitchValue.h",
    "ExK2Node_WaitCondition.h": "K2Nodes/ExK2Node_WaitCondition.h",
    "ExK2Node_WaitAll.h": "K2Nodes/ExK2Node_WaitAll.h",
    "ExK2Node_WaitAny.h": "K2Nodes/ExK2Node_WaitAny.h",
    "ExK2Node_WaitCount.h": "K2Nodes/ExK2Node_WaitCount.h",
    "ExK2Node_LoadAsset.h": "K2Nodes/ExK2Node_LoadAsset.h",
    "ExK2Node_StreamLevel.h": "K2Nodes/ExK2Node_StreamLevel.h",
    "ExK2Node_LoopDelay.h": "K2Nodes/ExK2Node_LoopDelay.h",
    "ExK2Node_ForLoopWithDelay.h": "K2Nodes/ExK2Node_ForLoopWithDelay.h",
    "ExK2Node_AsyncBlendPercent.h": "K2Nodes/ExK2Node_AsyncBlendPercent.h",
    "ExK2Node_GameplayTag.h": "K2Nodes/ExK2Node_GameplayTag.h",
    "ExK2Node_LatentTaskObject.h": "K2Nodes/ExK2Node_LatentTaskObject.h",
    "ExK2Node_LatentTaskCall.h": "K2Nodes/ExK2Node_LatentTaskCall.h",
    "ExK2Node_CreateTaskAsync.h": "K2Nodes/ExK2Node_CreateTaskAsync.h",
    "SGraphNode_ShowBase.h": "Slate/SGraphNode_ShowBase.h",
    "SGraphNode_ExAsyncDebug.h": "Slate/SGraphNode_ExAsyncDebug.h",
    "ExAssetTypeActions.h": "AssetActions/ExAssetTypeActions.h",
    "ExAssetTypeActions_FlowGraph.h": "AssetActions/ExAssetTypeActions_FlowGraph.h",
}
FILE_MAP_EDITOR_CPP = {k.replace(".h", ".cpp"): v.replace(".h", ".cpp") for k, v in FILE_MAP_EDITOR.items()}

# Class renames (longest first)
CLASS_RENAMES = [
    ("UExAsyncBlendPercentProxy", "UExProxy_BlendPercent"),
    ("UExForLoopWithDelayProxy", "UExProxy_ForLoopWithDelay"),
    ("UExWaitConditionProxy", "UExProxy_WaitCondition"),
    ("UExGameplayTagListenerProxy", "UExAsyncAction_GameplayTagListener"),
    ("UExGameplayTagModifierProxy", "UExAsyncAction_GameplayTagModifier"),
    ("UExGameplayTagQueryProxy", "UExAsyncAction_GameplayTagQuery"),
    ("UExAsyncLoadAssetCallbackProxy", "UExAsyncAction_LoadAssetCallbackProxy"),
    ("UExLatentTaskUUIDProxy", "UExLatentTask_BranchSync"),
    ("UExProxy_LatentTaskUUID", "UExLatentTask_BranchSync"),
    ("UExLatentActionProxyBase", "UExBase_FlowProxy"),
    ("UExLatentActionProxy", "UExAsyncAction_BranchSync"),
    ("UExAsyncLoadAsset", "UExAsyncAction_LoadAsset"),
    ("UExAsyncStreamLevel", "UExAsyncAction_StreamLevel"),
    ("UExLatentSpawnProxy", "UExAsyncAction_SpawnActor"),
    ("UExReplicationProxy", "UExAsyncAction_Replication"),
    ("UExSaveableLatentTask", "UExLatentTask_Saveable"),
    ("UExCheckpointProxy", "UExAsyncAction_Checkpoint"),
    ("UExWaitBranchProxy", "UExProxy_WaitBranch"),
    ("UExLatentTaskForAttach", "UExLatentTask_ForAttach"),
    ("UExLoopDelayProxy", "UExProxy_LoopDelay"),
    ("UExSaveGameProxy", "UExAsyncAction_SaveGame"),
    ("UExLatentTaskProxy", "UExLatentTask_Custom"),
    ("UExProxy_LatentTask", "UExLatentTask_Custom"),
    ("UExLatentTask_Graph", "UExLatentTask_Custom"),
    ("UExAsyncActionBase", "UExBase_AsyncAction"),
    ("UExLatentTaskBase", "UExBase_LatentTask"),
    ("IExLatentTaskInterface", "IExLatentTaskInterface"),  # no-op anchor
]

# Include path: old -> new (full BlueprintTool/ path)
def build_include_map():
    m = {}
    all_maps = {**FILE_MAP_RUNTIME, **FILE_MAP_RUNTIME_CPP, **FILE_MAP_EDITOR, **FILE_MAP_EDITOR_CPP}
    for old, new in all_maps.items():
        m[f"BlueprintTool/{old}"] = f"BlueprintTool/{new}"
        m[old] = f"BlueprintTool/{new}"
    # ExBase_FlowProxy is new
    m["BlueprintTool/ExLatentActionManager.h"] = "BlueprintTool/Proxies/ExBase_FlowProxy.h"
    m["ExLatentActionManager.h"] = "BlueprintTool/Subsystems/ExLatentActionManager.h"
    return m


INCLUDE_MAP = build_include_map()


def apply_class_renames(text: str) -> str:
    for old, new in CLASS_RENAMES:
        if old != new:
            text = text.replace(old, new)
    return text


def apply_include_updates(text: str) -> str:
    # Sort by length descending to avoid partial replacements
    for old, new in sorted(INCLUDE_MAP.items(), key=lambda x: -len(x[0])):
        text = text.replace(f'#include "{old}"', f'#include "{new}"')
    return text


def apply_generated_h(text: str, new_basename: str) -> str:
    stem = Path(new_basename).stem
    text = re.sub(r'#include "[^"]+\.generated\.h"', f'#include "{stem}.generated.h"', text, count=1)
    return text


def transform_content(content: str, new_rel_path: str) -> str:
    content = apply_class_renames(content)
    content = apply_include_updates(content)
    content = apply_generated_h(content, Path(new_rel_path).name)
    return content


def find_old_files(module: str, visibility: str):
    base = SOURCE_ROOT / module / visibility / "BlueprintTool"
    if not base.exists():
        return []
    return list(base.glob("*"))


def split_latent_action_manager():
    """Create ExBase_FlowProxy.h from original ExLatentActionManager.h."""
    src_h = SOURCE_ROOT / "BlueprintNodeGraph/Public/BlueprintTool/ExLatentActionManager.h"
    if not src_h.exists():
        return

    content = src_h.read_text(encoding="utf-8")

    # Extract proxy classes (lines before UExLatentActionManager)
    proxy_header = '''// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "ExBase_FlowProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDelayCompletedDelegate);

'''

    # Parse original - take from first UCLASS for UExLatentActionProxy through end of UExLatentActionProxyBase class
    start = content.find("/**\n * @class UExLatentActionProxy")
    end = content.find("/**\n * @class UExLatentActionManager")
    proxy_body = content[start:end]
    proxy_body = apply_class_renames(proxy_body)

    dst_h = SOURCE_ROOT / "BlueprintNodeGraph/Public/BlueprintTool/Proxies/ExBase_FlowProxy.h"
    dst_h.parent.mkdir(parents=True, exist_ok=True)
    dst_h.write_text(proxy_header + proxy_body, encoding="utf-8")

    # Manager-only header
    manager_header = '''// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/Proxies/ExBase_FlowProxy.h"
#include "BlueprintTool/Common/ExSubsystemGetter.h"
#include "Kismet/GameplayStatics.h"
#include "ExLatentActionManager.generated.h"

'''

    manager_start = content.find("/**\n * @class UExLatentActionManager")
    manager_body = content[manager_start:]
    manager_body = apply_class_renames(manager_body)

    dst_mgr = SOURCE_ROOT / "BlueprintNodeGraph/Public/BlueprintTool/Subsystems/ExLatentActionManager.h"
    dst_mgr.parent.mkdir(parents=True, exist_ok=True)
    dst_mgr.write_text(manager_header + manager_body, encoding="utf-8")

    # Split cpp
    src_cpp = SOURCE_ROOT / "BlueprintNodeGraph/Private/BlueprintTool/ExLatentActionManager.cpp"
    if src_cpp.exists():
        cpp = src_cpp.read_text(encoding="utf-8")
        cpp = apply_class_renames(cpp)
        cpp = apply_include_updates(cpp)

        proxy_cpp = cpp.replace(
            '#include "BlueprintTool/Subsystems/ExLatentActionManager.h"',
            '#include "BlueprintTool/Proxies/ExBase_FlowProxy.h"'
        )
        # Remove manager-specific if any (there isn't)

        dst_proxy_cpp = SOURCE_ROOT / "BlueprintNodeGraph/Private/BlueprintTool/Proxies/ExBase_FlowProxy.cpp"
        dst_proxy_cpp.parent.mkdir(parents=True, exist_ok=True)
        dst_proxy_cpp.write_text(
            proxy_cpp.replace(
                '#include "BlueprintTool/Common/ExBlueprintDebugBubble.h"',
                '#include "BlueprintTool/Common/ExBlueprintDebugBubble.h"\n#include "BlueprintTool/Subsystems/ExLatentActionManager.h"'
            ),
            encoding="utf-8",
        )

        # Empty manager cpp (templates are header-only)
        dst_mgr_cpp = SOURCE_ROOT / "BlueprintNodeGraph/Private/BlueprintTool/Subsystems/ExLatentActionManager.cpp"
        dst_mgr_cpp.parent.mkdir(parents=True, exist_ok=True)
        dst_mgr_cpp.write_text(
            '#include "BlueprintTool/Subsystems/ExLatentActionManager.h"\n',
            encoding="utf-8",
        )

    # Remove old files
    for p in [src_h, src_cpp]:
        if p.exists():
            p.unlink()
            print(f"  removed old {p.name}")


def migrate_module(module: str, file_map: dict, visibility: str):
    base = SOURCE_ROOT / module / visibility / "BlueprintTool"
    for old_name, new_rel in file_map.items():
        old_path = base / old_name
        if not old_path.exists():
            continue
        new_path = base / new_rel
        new_path.parent.mkdir(parents=True, exist_ok=True)

        content = old_path.read_text(encoding="utf-8")
        content = transform_content(content, new_rel)
        new_path.write_text(content, encoding="utf-8")
        old_path.unlink()
        print(f"  {module}/{visibility}: {old_name} -> {new_rel}")


def global_pass():
    """Update all remaining files in plugin source."""
    extensions = {".h", ".cpp", ".cs", ".md"}
    for root, _, files in os.walk(PLUGIN_ROOT):
        if "Intermediate" in root or "Scripts" in root:
            continue
        for f in files:
            if Path(f).suffix not in extensions:
                continue
            path = Path(root) / f
            try:
                content = path.read_text(encoding="utf-8")
            except (UnicodeDecodeError, PermissionError):
                continue
            new_content = apply_class_renames(content)
            new_content = apply_include_updates(new_content)
            if new_content != content:
                path.write_text(new_content, encoding="utf-8")
                print(f"  updated refs: {path.relative_to(PLUGIN_ROOT)}")


def cleanup_empty_dirs():
    for module in ["BlueprintNodeGraph", "BlueprintNodeGraphEditor"]:
        for vis in ["Public", "Private"]:
            base = SOURCE_ROOT / module / vis / "BlueprintTool"
            if base.exists():
                # remove leftover old files at root that were migrated
                for f in base.iterdir():
                    if f.is_file() and f.name.startswith("Ex"):
                        print(f"  WARNING: leftover {f}")


def main():
    print("=== Split ExLatentActionManager ===")
    split_latent_action_manager()

    print("\n=== Migrate Runtime Public ===")
    migrate_module("BlueprintNodeGraph", FILE_MAP_RUNTIME, "Public")

    print("\n=== Migrate Runtime Private ===")
    migrate_module("BlueprintNodeGraph", FILE_MAP_RUNTIME_CPP, "Private")

    print("\n=== Migrate Editor Public ===")
    migrate_module("BlueprintNodeGraphEditor", FILE_MAP_EDITOR, "Public")

    print("\n=== Migrate Editor Private ===")
    migrate_module("BlueprintNodeGraphEditor", FILE_MAP_EDITOR_CPP, "Private")

    print("\n=== Global reference pass ===")
    global_pass()

    print("\n=== Cleanup check ===")
    cleanup_empty_dirs()

    print("\nDone.")


if __name__ == "__main__":
    main()
