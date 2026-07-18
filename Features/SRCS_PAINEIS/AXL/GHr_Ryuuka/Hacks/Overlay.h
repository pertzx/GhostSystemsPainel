#pragma once

bool isOutsideScreen(ImVec2 pos, ImVec2 screen) {
    if (pos.y < 0) {
        return true;
    }
    if (pos.x > screen.x) {
        return true;
    }
    if (pos.y > screen.y) {
        return true;
    }
    return pos.x < 0;
}

ImVec2 pushToScreenBorder(ImVec2 Pos, ImVec2 screen, int offset) {
    int x = (int) Pos.x;
    int y = (int) Pos.y;
	
    if (Pos.y < 0) {
        y = -offset;
    }
	
    if (Pos.x > screen.x) {
        x = (int) screen.x + offset;
    }
	
    if (Pos.y > screen.y) {
        y = (int) screen.y + offset;
    }
	
    if (Pos.x < 0) {
        x = -offset;
    }
    return ImVec2(x, y);
}

void DrawCircleHealth(ImVec2 position, int health, int max_health, float radius) {
    float a_max = ((3.14159265359f * 2.0f));
    ImU32 healthColor = IM_COL32(45, 180, 45, 255);
    if (health <= (max_health * 0.6)) {
        healthColor = IM_COL32(180, 180, 45, 255);
    }
    if (health < (max_health * 0.3)) {
        healthColor = IM_COL32(180, 45, 45, 255);
    }
    ImGui::GetForegroundDrawList()->PathArcTo(position, radius, (-(a_max / 4.0f)) + (a_max / max_health) * (max_health - health), a_max - (a_max / 4.0f));
    ImGui::GetForegroundDrawList()->PathStroke(healthColor, ImDrawFlags_None, 4);
}

void DrawESP(ImDrawList *draw) {
	
	if (!Config.InitImGui.bInitDone)
		return;
		
	if (Enable_ESP) {
		
		static ActorManager *get_actorManager = KyriosFramework::get_actorManager();
		if (get_actorManager != nullptr) {
			
			List<ActorLinker *> *GetAllHeros = get_actorManager->GetAllHeros();
			if (GetAllHeros != nullptr) {
				
				Vector3 myPos = Vector3::zero();
				
				ActorLinker *myActorLinker = GetAllHeros->getItems()[1];
				if (myActorLinker != nullptr) {
					myPos = myActorLinker->get_position();
				}
				
				Vector3 myPosSC = Camera::get_main()->WorldToScreenPoint(myPos);
				ImVec2 myPos_Vec2 = ImVec2(glWidth - myPosSC.x, myPosSC.y);
				
				if (myPosSC.z > 0) {
					myPos_Vec2 = ImVec2(myPosSC.x, glHeight - myPosSC.y);
				}
				
				ActorLinker **actorLinkers = (ActorLinker **) GetAllHeros->getItems();
				for (int i = 0; i < GetAllHeros->getSize(); i++) {
					
					ActorLinker *actorLinker = actorLinkers[(i *2) + 1];
					if (actorLinker) {
						
						Vector3 rootPos_W2S = Camera::get_main()->WorldToScreenPoint(actorLinker->get_position());
						Vector2 rootPos_Vec2 = Vector2(glWidth - rootPos_W2S.x, rootPos_W2S.y);
						
						if (rootPos_W2S.z > 0) {
							rootPos_Vec2 = Vector2(rootPos_W2S.x, glHeight - rootPos_W2S.y);
						}
						
						Vector2 headPos_Vec2 = Vector2(rootPos_Vec2.x, rootPos_Vec2.y - (glHeight / 6.35));
						float distanceToMe = Vector3::Distance(myPos, actorLinker->get_position());
						
						if (!actorLinker->IsHostCamp() && !actorLinker->ValueComponent()->get_actorHp() < 1) {
							
							if (PlayerLine) {
								draw->AddLine(myPos_Vec2, ImVec2(rootPos_Vec2.x, rootPos_Vec2.y), IM_COL32(45, 180, 45, 255), 1.7f);
							}
							
							if (PlayerBox) {
								float boxHeight = abs(headPos_Vec2.y - rootPos_Vec2.y);
								float boxWidth = boxHeight * 0.75f;
								
								ImVec2 vStart = {headPos_Vec2.x - (boxWidth / 2), headPos_Vec2.y};
								ImVec2 vEnd = {vStart.x + boxWidth, vStart.y + boxHeight};
								draw->AddRect(vStart, vEnd, IM_COL32(45, 180, 45, 255), 0, 240, 1.7f);
							}
							
							if (PlayerHealth) {
								float boxHeight = abs(headPos_Vec2.y - rootPos_Vec2.y);
								float boxWidth = boxHeight * 0.75f;
								
								float PercentHP = ((float)actorLinker->ValueComponent()->get_actorHp() * boxHeight) / (float)actorLinker->ValueComponent()->get_actorHpTotal();
								draw->AddRectFilled(ImVec2(rootPos_Vec2.x + (boxWidth / 2) + 5, rootPos_Vec2.y), ImVec2(rootPos_Vec2.x + (boxWidth / 2) + 15, rootPos_Vec2.y - PercentHP), IM_COL32(45, 180, 45, 255));
								draw->AddRect(ImVec2(rootPos_Vec2.x + (boxWidth / 2) + 5, rootPos_Vec2.y), ImVec2(headPos_Vec2.x + (boxWidth / 2) + 15, headPos_Vec2.y), IM_COL32(0, 0, 0, 255), 0, 240, 0.5);
							}
							
							if (PlayerName) {
								
								std::string strName = actorLinker->CharInfo()->ActorName()->CString();
								
								auto textSize = ImGui::CalcTextSize(strName.c_str(), 0, ((float) glHeight / 39.0f));
								draw->AddText(NULL, ((float) glHeight / 39.0f), {rootPos_Vec2.x - (textSize.x / 2), rootPos_Vec2.y + 25}, IM_COL32(255, 255, 255, 255), strName.c_str());
							}
							
							if (PlayerDistance) {
								std::string strDistance = to_string((int) distanceToMe); + " M";
								
								auto textSize = ImGui::CalcTextSize(strDistance.c_str(), 0, ((float) glHeight / 39.0f));
								draw->AddText(NULL, ((float) glHeight / 39.0f), {rootPos_Vec2.x - (textSize.x / 2), rootPos_Vec2.y + 5}, IM_COL32(0, 180, 255, 255), strDistance.c_str());
							}
							
							if (PlayerAlert && isOutsideScreen(ImVec2(rootPos_Vec2.x, rootPos_Vec2.y), ImVec2(glWidth, glHeight))) {
								ImVec2 hintDotRenderPos = pushToScreenBorder(ImVec2(rootPos_Vec2.x, rootPos_Vec2.y), ImVec2(glWidth, glHeight), - 50);
								ImVec2 hintTextRenderPos = pushToScreenBorder(ImVec2(rootPos_Vec2.x, rootPos_Vec2.y), ImVec2(glWidth, glHeight), - 50);
								
								draw->AddCircleFilled(hintDotRenderPos, 25, IM_COL32(255, 0, 0, 110));
								DrawCircleHealth(hintDotRenderPos, actorLinker->ValueComponent()->get_actorHp(), actorLinker->ValueComponent()->get_actorHpTotal(), 25);
								
								std::string strDistance = to_string((int) distanceToMe) + " M";
								auto textSize = ImGui::CalcTextSize(strDistance.c_str(), 0, ((float) glHeight / 45.0f));
								draw->AddText(NULL, ((float) glHeight / 45.0f), {hintTextRenderPos.x - (textSize.x / 2), hintTextRenderPos.y - 7}, IM_COL32(255, 255, 255, 255), strDistance.c_str());
								
								if (actorLinker->CharInfo()->ActorName()) {
									std::string strName = actorLinker->CharInfo()->ActorName()->CString();
									auto textSize = ImGui::CalcTextSize(strName.c_str(), 0, ((float) glHeight / 39.0f));
									draw->AddText(NULL, ((float) glHeight / 39.0f), {hintTextRenderPos.x - (textSize.x / 2), hintTextRenderPos.y + 30}, IM_COL32(255, 255, 255, 255), strName.c_str());
								}
							}
						}
					}
				}
			}
			
		}
	}
}