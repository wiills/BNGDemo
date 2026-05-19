# -*- coding: utf-8 -*-
from pathlib import Path

p = Path(__file__).resolve().parents[1] / "Source/BlueprintNodeGraphEditor/Private/BlueprintTool/K2Nodes/ExK2Node_QuestTask.cpp"
text = p.read_text(encoding="utf-8", errors="replace")
old_marker = "FText UExK2Node_QuestTask::GetTooltipText() const"
start = text.index(old_marker)
end = text.index("}", start) + 1
replacement = """FText UExK2Node_QuestTask::GetTooltipText() const
{
	return LOCTEXT("QuestTaskNodeTooltip", TEXT("Create a quest-bound latent task; updates quest progress on successful completion. Do not use Create Latent Task."));
}"""
text = text[:start] + replacement + text[end:]
p.write_text(text, encoding="utf-8")
print("fixed tooltip")
