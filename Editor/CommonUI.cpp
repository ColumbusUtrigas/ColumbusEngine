#include <Editor/CommonUI.h>
#include <Editor/Icons.h>
#include <Core/Reflection.h>
#include <Core/Filesystem.h>
#include <Core/Util.h>
#include <Math/InterpolationCurve.h>
#include <Math/Quaternion.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Scene/Project.h>
#include <Graphics/DebugUI.h>
#include <Graphics/World.h>
#include <imgui_internal.h>
#include <Lib/imgui/misc/cpp/imgui_stdlib.h>
#include <Lib/implot/implot.h>
#include <Lib/nativefiledialog/src/include/nfd.h>

#include <cfloat>
#include <cmath>
#include <filesystem>
#include <type_traits>
#include <unordered_map>


namespace Columbus::Editor
{
	_CommonUISettings CommonUISettings;

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Helper for UI property metadata
	struct FieldUIMetadata
	{
		bool NoEdit = false;
		bool Hidden = false;
		bool NoArrayResize = false;
		bool CommitOnEnter = false;
		bool HasSliderLimits = false;
		float SliderMin = 0.0f;
		float SliderMax = 0.0f;
		bool IsPicker = false;
		bool IsColor = false;
		bool IsHDR = false;
		bool UseChannelColours = false;
		bool IsGradient = false;
	};

	static FieldUIMetadata ParseFieldMetadata(const char* Meta)
	{
		FieldUIMetadata Result;

		if (!Meta)
			return Result;

		// Parse slider limits
		const char* SliderMin = strstr(Meta, "SliderMin(");
		if (SliderMin)
		{
			SliderMin += 10; // Skip "SliderMin("
			Result.SliderMin = (float)atof(SliderMin);
			Result.HasSliderLimits = true;
		}

		const char* SliderMax = strstr(Meta, "SliderMax(");
		if (SliderMax)
		{
			SliderMax += 10; // Skip "SliderMax("
			Result.SliderMax = (float)atof(SliderMax);
			Result.HasSliderLimits = true;
		}

		// Parse flags
		Result.NoEdit = strstr(Meta, "Noedit") != nullptr;
		Result.Hidden = strstr(Meta, "Hidden") != nullptr;
		Result.NoArrayResize = strstr(Meta, "NoArrayResize") != nullptr;
		Result.CommitOnEnter = strstr(Meta, "EnterCommits") != nullptr;
		Result.IsPicker = strstr(Meta, "Picker") != nullptr;
		Result.IsColor = strstr(Meta, "Colour") != nullptr && strstr(Meta, "ColourChannels") == nullptr;
		Result.IsHDR = strstr(Meta, "HDR") != nullptr;
		Result.UseChannelColours = strstr(Meta, "ColourChannels") != nullptr;
		Result.IsGradient = strstr(Meta, "Gradient") != nullptr;

		return Result;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////
	// Custom UI specialisations for reflected types

	struct CurvePlotViewState
	{
		float MinValue = -1.0f;
		float MaxValue = 1.0f;
		bool bInitialised = false;
		bool bWasDraggingPoint = false;
	};

	struct CurveEditorState
	{
		int SelectedIndex = -1;
		int ActiveGradientStop = -1;
		CurvePlotViewState PlotView;
	};

	static std::unordered_map<ImGuiID, CurveEditorState> GCurveEditorStates;
	static ImGuiID GActiveCurveEditorId = 0;

	static constexpr const char* GVectorChannelLabels[] = { "X", "Y", "Z", "W" };
	static constexpr const char* GColorChannelLabels[] = { "R", "G", "B", "A" };

	static ImVec4 GetCurveChannelColor(int Component, bool bIsColorCurve)
	{
		if (bIsColorCurve)
		{
			switch (Component)
			{
				case 0: return ImVec4(0.95f, 0.35f, 0.35f, 1.0f);
				case 1: return ImVec4(0.35f, 0.90f, 0.45f, 1.0f);
				case 2: return ImVec4(0.35f, 0.65f, 1.0f, 1.0f);
				default: return ImVec4(0.92f, 0.92f, 0.92f, 1.0f);
			}
		}

		switch (Component)
		{
			case 0: return ImVec4(0.95f, 0.42f, 0.42f, 1.0f);
			case 1: return ImVec4(0.45f, 0.88f, 0.48f, 1.0f);
			case 2: return ImVec4(0.42f, 0.68f, 1.0f, 1.0f);
			default: return ImVec4(0.88f, 0.82f, 0.42f, 1.0f);
		}
	}

	static ImVec4 ToImVec4(const Vector3& Value)
	{
		return ImVec4(Value.X, Value.Y, Value.Z, 1.0f);
	}

	static ImVec4 ToImVec4(const Vector4& Value)
	{
		return ImVec4(Value.X, Value.Y, Value.Z, Value.W);
	}

	static bool DrawStyledFloatField(const char* Label, float& Value, const ImVec4* Color, bool bDragEdit)
	{
		if (Color)
		{
			const ImVec4& Tint = *Color;
			ImGui::PushStyleColor(ImGuiCol_Text, Tint);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(Tint.x * 0.22f, Tint.y * 0.22f, Tint.z * 0.22f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(Tint.x * 0.33f, Tint.y * 0.33f, Tint.z * 0.33f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(Tint.x * 0.44f, Tint.y * 0.44f, Tint.z * 0.44f, 1.0f));
		}

		const bool Changed = bDragEdit
			? ImGui::DragFloat(Label, &Value, 0.01f, 0.0f, 0.0f, "%.3f")
			: ImGui::InputFloat(Label, &Value, 0.0f, 0.0f, "%.3f");

		if (Color)
			ImGui::PopStyleColor(4);

		return Changed;
	}

	static bool DrawInlineFloatComponents(const char* Label, float* Values, int ComponentCount, const char* const* ComponentLabels, bool bUseChannelColours, bool bDragEdit, bool bIsColorSemantic = false)
	{
		bool Changed = false;
		ImGui::PushID(Label ? Label : "InlineFloatComponents");

		ImGui::PushMultiItemsWidths(ComponentCount, ImGui::CalcItemWidth());
		for (int Component = 0; Component < ComponentCount; Component++)
		{
			ImGui::PushID(Component);
			if (Component > 0)
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);

			ImGui::SetNextItemWidth(ImGui::CalcItemWidth());
			const ImVec4 Color = GetCurveChannelColor(Component, bIsColorSemantic);
			const ImVec4* ColorPtr = bUseChannelColours ? &Color : nullptr;
			std::string ComponentId = std::string("##") + ComponentLabels[Component];
			Changed |= DrawStyledFloatField(ComponentId.c_str(), Values[Component], ColorPtr, bDragEdit);
			ImGui::PopItemWidth();
			ImGui::PopID();
		}

		if (Label && Label[0] != '\0')
		{
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(Label);
		}

		ImGui::PopID();
		return Changed;
	}

	static std::string GetArrayElementHeaderLabel(const Reflection::Struct* Struct, const void* ElementData, int Index)
	{
		std::string DisplayLabel;
		if (Struct && Struct->ArrayElementLabel)
		{
			DisplayLabel = Struct->ArrayElementLabel(ElementData, Index);
		}

		if (DisplayLabel.empty())
		{
			const char* StructName = Struct && Struct->Name ? Struct->Name : "Element";
			DisplayLabel = std::string(StructName) + " " + std::to_string(Index);
		}

		DisplayLabel += "##ArrayElement";
		DisplayLabel += std::to_string(Index);
		return DisplayLabel;
	}

	static float& GetCurveValueComponent(float& Value, int Component)
	{
		return Value;
	}

	static const float& GetCurveValueComponent(const float& Value, int Component)
	{
		return Value;
	}

	static float& GetCurveValueComponent(Vector2& Value, int Component)
	{
		return Component == 0 ? Value.X : Value.Y;
	}

	static const float& GetCurveValueComponent(const Vector2& Value, int Component)
	{
		return Component == 0 ? Value.X : Value.Y;
	}

	static float& GetCurveValueComponent(Vector3& Value, int Component)
	{
		switch (Component)
		{
			case 0: return Value.X;
			case 1: return Value.Y;
			default: return Value.Z;
		}
	}

	static const float& GetCurveValueComponent(const Vector3& Value, int Component)
	{
		switch (Component)
		{
			case 0: return Value.X;
			case 1: return Value.Y;
			default: return Value.Z;
		}
	}

	static float& GetCurveValueComponent(Vector4& Value, int Component)
	{
		switch (Component)
		{
			case 0: return Value.X;
			case 1: return Value.Y;
			case 2: return Value.Z;
			default: return Value.W;
		}
	}

	static const float& GetCurveValueComponent(const Vector4& Value, int Component)
	{
		switch (Component)
		{
			case 0: return Value.X;
			case 1: return Value.Y;
			case 2: return Value.Z;
			default: return Value.W;
		}
	}

	template <typename ValueType>
	static constexpr int GetCurveComponentCount()
	{
		if constexpr (std::is_same_v<ValueType, float>)
			return 1;
		else if constexpr (std::is_same_v<ValueType, Vector2>)
			return 2;
		else if constexpr (std::is_same_v<ValueType, Vector3>)
			return 3;
		else
			return 4;
	}

	template <typename ValueType>
	static const char* GetCurveNumericChannelLabel(int Component)
	{
		if constexpr (std::is_same_v<ValueType, float>)
			return "Value";
		else if constexpr (std::is_same_v<ValueType, Vector2>)
			return GVectorChannelLabels[Component];
		else if constexpr (std::is_same_v<ValueType, Vector3>)
			return GVectorChannelLabels[Component];
		else
			return GVectorChannelLabels[Component];
	}

	template <typename ValueType>
	static bool DrawCurveValueEditor(ValueType& Value, bool bIsColorCurve)
	{
		bool bChanged = false;
		const int ComponentCount = GetCurveComponentCount<ValueType>();

		for (int Component = 0; Component < ComponentCount; Component++)
		{
			if (Component > 0)
				ImGui::SameLine();

			ImGui::PushItemWidth(110.0f);
			float& ComponentValue = GetCurveValueComponent(Value, Component);
			const ImVec4 ChannelColor = GetCurveChannelColor(Component, bIsColorCurve);
			bChanged |= DrawStyledFloatField(GetCurveNumericChannelLabel<ValueType>(Component), ComponentValue, &ChannelColor, true);
			ImGui::PopItemWidth();
		}

		return bChanged;
	}

	static void UpdateCurvePlotViewRange(CurvePlotViewState& View, float TargetMin, float TargetMax, bool bAllowChange)
	{
		if (!View.bInitialised)
		{
			View.MinValue = TargetMin;
			View.MaxValue = TargetMax;
			View.bInitialised = true;
			return;
		}

		if (bAllowChange)
		{
			View.MinValue = TargetMin;
			View.MaxValue = TargetMax;
		}
	}

	template <typename CurveType>
	static void ComputeCurveValueRange(const CurveType& Curve, float& OutMinValue, float& OutMaxValue)
	{
		using CurveValueType = std::decay_t<decltype(std::declval<CurveType>().Points[0].Value)>;
		const int ComponentCount = GetCurveComponentCount<CurveValueType>();

		OutMinValue = FLT_MAX;
		OutMaxValue = -FLT_MAX;

		for (const auto& Point : Curve.Points)
		{
			for (int Component = 0; Component < ComponentCount; Component++)
			{
				const float Value = GetCurveValueComponent(Point.Value, Component);
				OutMinValue = std::min(OutMinValue, Value);
				OutMaxValue = std::max(OutMaxValue, Value);
			}
		}

		if (OutMinValue == FLT_MAX || OutMaxValue == -FLT_MAX)
		{
			OutMinValue = -1.0f;
			OutMaxValue = 1.0f;
			return;
		}

		if (fabsf(OutMaxValue - OutMinValue) < 0.1f)
		{
			OutMinValue -= 0.5f;
			OutMaxValue += 0.5f;
		}
		else
		{
			const float Padding = (OutMaxValue - OutMinValue) * 0.15f;
			OutMinValue -= Padding;
			OutMaxValue += Padding;
		}
	}

	template <typename CurveType>
	static int FindClosestCurvePointIndex(const CurveType& Curve, float Key)
	{
		if (Curve.Points.empty())
			return -1;

		int BestIndex = 0;
		float BestDistance = fabsf(Curve.Points[0].Key - Key);
		for (int i = 1; i < (int)Curve.Points.size(); i++)
		{
			const float Distance = fabsf(Curve.Points[i].Key - Key);
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestIndex = i;
			}
		}

		return BestIndex;
	}

	template <typename CurveType>
	static int AddCurvePoint(CurveType& Curve, int SelectedIndex)
	{
		float NewKey = 0.5f;
		if (!Curve.Points.empty())
		{
			if (SelectedIndex >= 0 && SelectedIndex < (int)Curve.Points.size())
			{
				if (SelectedIndex + 1 < (int)Curve.Points.size())
					NewKey = 0.5f * (Curve.Points[SelectedIndex].Key + Curve.Points[SelectedIndex + 1].Key);
				else if (SelectedIndex > 0)
					NewKey = 0.5f * (Curve.Points[SelectedIndex].Key + 1.0f);
				else
					NewKey = std::clamp(Curve.Points[SelectedIndex].Key + 0.1f, 0.0f, 1.0f);
			}
		}

		const auto NewValue = Curve.Interpolate(NewKey);
		Curve.Points.emplace_back(NewKey, NewValue);
		Curve.Sort();
		return FindClosestCurvePointIndex(Curve, NewKey);
	}

	template <typename CurveType>
	static void RemoveCurvePoint(CurveType& Curve, int& SelectedIndex)
	{
		if (SelectedIndex < 0 || SelectedIndex >= (int)Curve.Points.size())
			return;

		Curve.RemovePoint((size_t)SelectedIndex);
		if (Curve.Points.empty())
			SelectedIndex = -1;
		else if (SelectedIndex >= (int)Curve.Points.size())
			SelectedIndex = (int)Curve.Points.size() - 1;
	}

	template <typename CurveType>
	static void SetFlatCurvePreset(CurveType& Curve, int& SelectedIndex)
	{
		using CurveValueType = std::decay_t<decltype(std::declval<CurveType>().Points[0].Value)>;
		Curve.Points.clear();
		Curve.Points.emplace_back(0.0f, CurveValueType(1.0f));
		Curve.Points.emplace_back(1.0f, CurveValueType(1.0f));
		SelectedIndex = 0;
	}

	template <typename CurveType>
	static void FitCurvePlotView(CurveType& Curve, CurveEditorState& State)
	{
		float FitMin = -1.0f;
		float FitMax = 1.0f;
		ComputeCurveValueRange(Curve, FitMin, FitMax);
		UpdateCurvePlotViewRange(State.PlotView, FitMin, FitMax, true);
	}

	static const char* GetCurveInterpolationModeLabel(EInterpolationCurveMode Mode)
	{
		switch (Mode)
		{
			case EInterpolationCurveMode::Linear:   return "Linear";
			case EInterpolationCurveMode::Bezier:   return "Bezier";
			case EInterpolationCurveMode::Constant: return "Constant";
			default:                                return "Linear";
		}
	}

	template <typename CurveType>
	static bool DrawCurveEditorToolbar(const char* RemoveLabel, const char* FlatPresetLabel, CurveType& Curve, CurveEditorState& State, ImGuiID EditorId)
	{
		bool Changed = false;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));

		if (ImGui::Button(ICON_FA_PLUS " Add Point"))
		{
			GActiveCurveEditorId = EditorId;
			State.SelectedIndex = AddCurvePoint(Curve, State.SelectedIndex);
			FitCurvePlotView(Curve, State);
			Changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(RemoveLabel))
		{
			GActiveCurveEditorId = EditorId;
			RemoveCurvePoint(Curve, State.SelectedIndex);
			FitCurvePlotView(Curve, State);
			Changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(FlatPresetLabel))
		{
			GActiveCurveEditorId = EditorId;
			SetFlatCurvePreset(Curve, State.SelectedIndex);
			FitCurvePlotView(Curve, State);
			Changed = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(160.0f);
		int ModeIndex = (int)Curve.Mode;
		if (ImGui::Combo("##InterpolationMode", &ModeIndex, "Linear\0Bezier\0Constant\0"))
		{
			GActiveCurveEditorId = EditorId;
			Curve.Mode = (EInterpolationCurveMode)ModeIndex;
			Changed = true;
		}
		ImGui::SameLine();
		ImGui::TextDisabled("%s", GetCurveInterpolationModeLabel(Curve.Mode));
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_EXPAND " Fit"))
		{
			GActiveCurveEditorId = EditorId;
			FitCurvePlotView(Curve, State);
		}

		ImGui::PopStyleVar();
		return Changed;
	}

	template <typename CurveType>
	static bool HandleCurveDeleteShortcut(CurveType& Curve, CurveEditorState& State, ImGuiID EditorId)
	{
		if (GActiveCurveEditorId != EditorId
			|| !ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
			|| State.SelectedIndex < 0
			|| State.SelectedIndex >= (int)Curve.Points.size()
			|| ImGui::IsAnyItemActive()
			|| !ImGui::IsKeyPressed(ImGuiKey_Delete, false))
		{
			return false;
		}

		RemoveCurvePoint(Curve, State.SelectedIndex);
		FitCurvePlotView(Curve, State);
		return true;
	}

	static void DrawColorGradientStopHandle(ImDrawList* DrawList, const ImRect& BarRect, const ImVec4& Color, float Key, bool bSelected)
	{
		const float HandleX = ImLerp(BarRect.Min.x, BarRect.Max.x, Key);
		const ImVec2 Tip(HandleX, BarRect.Max.y + 4.0f);
		const ImVec2 Left(HandleX - 7.0f, BarRect.Max.y + 18.0f);
		const ImVec2 Right(HandleX + 7.0f, BarRect.Max.y + 18.0f);
		const ImVec2 SquareMin(HandleX - 6.0f, BarRect.Max.y + 18.0f);
		const ImVec2 SquareMax(HandleX + 6.0f, BarRect.Max.y + 30.0f);

		const ImU32 FillColor = ImGui::ColorConvertFloat4ToU32(Color);
		const ImU32 BorderColor = ImGui::GetColorU32(bSelected ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
		const float Thickness = bSelected ? 2.0f : 1.0f;

		DrawList->AddTriangleFilled(Tip, Left, Right, FillColor);
		DrawList->AddRectFilled(SquareMin, SquareMax, FillColor, 2.0f);
		DrawList->AddTriangle(Tip, Left, Right, BorderColor, Thickness);
		DrawList->AddRect(SquareMin, SquareMax, BorderColor, 2.0f, 0, Thickness);
	}

	template <typename CurveType>
	static int FindHoveredColorGradientStop(const CurveType& Curve, const ImRect& BarRect, const ImVec2& MousePosition)
	{
		for (int PointIndex = 0; PointIndex < (int)Curve.Points.size(); PointIndex++)
		{
			const float HandleX = ImLerp(BarRect.Min.x, BarRect.Max.x, Curve.Points[PointIndex].Key);
			const ImRect HitRect(
				ImVec2(HandleX - 9.0f, BarRect.Max.y + 2.0f),
				ImVec2(HandleX + 9.0f, BarRect.Max.y + 32.0f));
			if (HitRect.Contains(MousePosition))
				return PointIndex;
		}

		return -1;
	}

	template <typename CurveType>
	static bool DrawNumericInterpolationCurveEditor(CurveType& Curve, const Reflection::Field& Field, CurveEditorState& State, ImGuiID EditorId)
	{
		using CurveValueType = std::decay_t<decltype(std::declval<CurveType>().Points[0].Value)>;
		constexpr float PointKeyPadding = 0.001f;
		const int ComponentCount = GetCurveComponentCount<CurveValueType>();

		if (State.SelectedIndex < 0 && !Curve.Points.empty())
			State.SelectedIndex = 0;

		bool Changed = false;
		ImGui::SeparatorText(Field.Name);
		Changed |= DrawCurveEditorToolbar(ICON_FA_TRASH " Remove Point", ICON_FA_STREAM " Flat 1", Curve, State, EditorId);
		Changed |= HandleCurveDeleteShortcut(Curve, State, EditorId);

		if (State.SelectedIndex >= (int)Curve.Points.size())
			State.SelectedIndex = (int)Curve.Points.size() - 1;

		float RangeMin = -1.0f;
		float RangeMax = 1.0f;
		ComputeCurveValueRange(Curve, RangeMin, RangeMax);
		UpdateCurvePlotViewRange(State.PlotView, RangeMin, RangeMax, false);

		if (ImPlot::BeginPlot("##CurvePlot", ImVec2(-1.0f, 220.0f), ImPlotFlags_NoMenus))
		{
			ImPlot::SetupAxes("Time", "Value");
			ImPlot::SetupAxesLimits(0.0, 1.0, State.PlotView.MinValue, State.PlotView.MaxValue, ImPlotCond_Always);
			ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_NoMenus);

			std::vector<double> Xs(Curve.Points.size());
			std::vector<double> Ys(Curve.Points.size());
			bool bAnyPointHeld = false;

			if (State.SelectedIndex >= 0 && State.SelectedIndex < (int)Curve.Points.size())
			{
				const double HighlightXs[2] = { Curve.Points[State.SelectedIndex].Key, Curve.Points[State.SelectedIndex].Key };
				const double HighlightYs[2] = { State.PlotView.MinValue, State.PlotView.MaxValue };
				ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 0.18f), 1.0f);
				ImPlot::PlotLine("##SelectedKeyLine", HighlightXs, HighlightYs, 2, ImPlotLineFlags_NoClip);
			}

			for (int Component = 0; Component < ComponentCount; Component++)
			{
				for (int PointIndex = 0; PointIndex < (int)Curve.Points.size(); PointIndex++)
				{
					Xs[PointIndex] = Curve.Points[PointIndex].Key;
					Ys[PointIndex] = GetCurveValueComponent(Curve.Points[PointIndex].Value, Component);
				}

				if (Curve.Points.size() >= 2)
				{
					if (Curve.Mode == EInterpolationCurveMode::Linear)
					{
						ImPlot::SetNextLineStyle(GetCurveChannelColor(Component, false), 2.0f);
						ImPlot::PlotLine(GetCurveNumericChannelLabel<CurveValueType>(Component), Xs.data(), Ys.data(), (int)Curve.Points.size());
					}
					else if (Curve.Mode == EInterpolationCurveMode::Constant)
					{
						std::vector<double> StepXs;
						std::vector<double> StepYs;
						StepXs.reserve(Curve.Points.size() * 2);
						StepYs.reserve(Curve.Points.size() * 2);

						StepXs.push_back(Xs[0]);
						StepYs.push_back(Ys[0]);
						for (int PointIndex = 1; PointIndex < (int)Curve.Points.size(); PointIndex++)
						{
							StepXs.push_back(Xs[PointIndex]);
							StepYs.push_back(Ys[PointIndex - 1]);
							StepXs.push_back(Xs[PointIndex]);
							StepYs.push_back(Ys[PointIndex]);
						}

						ImPlot::SetNextLineStyle(GetCurveChannelColor(Component, false), 2.0f);
						ImPlot::PlotLine(GetCurveNumericChannelLabel<CurveValueType>(Component), StepXs.data(), StepYs.data(), (int)StepXs.size());
					}
					else
					{
						const int SampleCount = 96;
						std::vector<double> SampleXs(SampleCount);
						std::vector<double> SampleYs(SampleCount);
						for (int SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++)
						{
							const float T = SampleCount > 1 ? (float)SampleIndex / (float)(SampleCount - 1) : 0.0f;
							SampleXs[SampleIndex] = T;
							SampleYs[SampleIndex] = GetCurveValueComponent(Curve.Interpolate(T), Component);
						}

						ImPlot::SetNextLineStyle(GetCurveChannelColor(Component, false), 2.0f);
						ImPlot::PlotLine(GetCurveNumericChannelLabel<CurveValueType>(Component), SampleXs.data(), SampleYs.data(), SampleCount);
					}
				}

				for (int PointIndex = 0; PointIndex < (int)Curve.Points.size(); PointIndex++)
				{
					double Key = Curve.Points[PointIndex].Key;
					double Value = GetCurveValueComponent(Curve.Points[PointIndex].Value, Component);
					bool bClicked = false;
					bool bHeld = false;
					const int DragPointId = (int)(EditorId + (ImGuiID)(PointIndex * 8 + Component + 1));
					ImPlot::DragPoint(
						DragPointId,
						&Key,
						&Value,
						GetCurveChannelColor(Component, false),
						PointIndex == State.SelectedIndex ? 7.0f : 5.0f,
						ImPlotDragToolFlags_NoFit,
						&bClicked,
						nullptr,
						&bHeld);

					if (bClicked || bHeld)
					{
						GActiveCurveEditorId = EditorId;
						const float MinKey = PointIndex > 0 ? Curve.Points[PointIndex - 1].Key + PointKeyPadding : 0.0f;
						const float MaxKey = PointIndex + 1 < (int)Curve.Points.size() ? Curve.Points[PointIndex + 1].Key - PointKeyPadding : 1.0f;
						Curve.Points[PointIndex].Key = (float)std::clamp(Key, (double)MinKey, (double)MaxKey);
						GetCurveValueComponent(Curve.Points[PointIndex].Value, Component) = (float)Value;
						State.SelectedIndex = PointIndex;
						Changed = true;
					}

					bAnyPointHeld |= bHeld;
				}
			}

			if (ImPlot::IsPlotHovered())
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
					GActiveCurveEditorId = EditorId;

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					GActiveCurveEditorId = EditorId;
					const ImPlotPoint MousePlotPosition = ImPlot::GetPlotMousePos();
					const float NewKey = std::clamp((float)MousePlotPosition.x, 0.0f, 1.0f);
					auto NewValue = Curve.Interpolate(NewKey);

					if constexpr (std::is_same_v<CurveValueType, float>)
					{
						NewValue = (float)MousePlotPosition.y;
					}
					else
					{
						int ClosestComponent = 0;
						float BestDistance = fabsf(GetCurveValueComponent(NewValue, 0) - (float)MousePlotPosition.y);
						for (int Component = 1; Component < ComponentCount; Component++)
						{
							const float Distance = fabsf(GetCurveValueComponent(NewValue, Component) - (float)MousePlotPosition.y);
							if (Distance < BestDistance)
							{
								BestDistance = Distance;
								ClosestComponent = Component;
							}
						}

						GetCurveValueComponent(NewValue, ClosestComponent) = (float)MousePlotPosition.y;
					}

					Curve.Points.emplace_back(NewKey, NewValue);
					Curve.Sort();
					State.SelectedIndex = FindClosestCurvePointIndex(Curve, NewKey);
					FitCurvePlotView(Curve, State);
					Changed = true;
				}
			}

			if (!bAnyPointHeld && State.PlotView.bWasDraggingPoint)
				FitCurvePlotView(Curve, State);

			State.PlotView.bWasDraggingPoint = bAnyPointHeld;
			ImPlot::EndPlot();
		}

		if (State.SelectedIndex >= 0 && State.SelectedIndex < (int)Curve.Points.size())
		{
			auto& Point = Curve.Points[State.SelectedIndex];
			ImGui::Spacing();
			ImGui::TextDisabled("Selected point %d", State.SelectedIndex);

			const float MinKey = State.SelectedIndex > 0 ? Curve.Points[State.SelectedIndex - 1].Key + PointKeyPadding : 0.0f;
			const float MaxKey = State.SelectedIndex + 1 < (int)Curve.Points.size() ? Curve.Points[State.SelectedIndex + 1].Key - PointKeyPadding : 1.0f;
			bool bPointChanged = false;
			bPointChanged |= ImGui::SliderFloat("Time", &Point.Key, MinKey, MaxKey, "%.3f");
			bPointChanged |= DrawCurveValueEditor(Point.Value, false);
			if (bPointChanged)
			{
				GActiveCurveEditorId = EditorId;
				FitCurvePlotView(Curve, State);
				Changed = true;
			}
		}

		return Changed;
	}

	template <typename CurveType>
	static bool DrawGradientInterpolationCurveEditor(CurveType& Curve, const Reflection::Field& Field, CurveEditorState& State, ImGuiID EditorId)
	{
		using CurveValueType = std::decay_t<decltype(std::declval<CurveType>().Points[0].Value)>;

		if (State.SelectedIndex < 0 && !Curve.Points.empty())
			State.SelectedIndex = 0;

		bool Changed = false;
		ImGui::SeparatorText(Field.Name);
		Changed |= DrawCurveEditorToolbar(ICON_FA_TRASH " Remove Stop", ICON_FA_STREAM " Flat 1", Curve, State, EditorId);
		Changed |= HandleCurveDeleteShortcut(Curve, State, EditorId);

		const float Width = ImGui::GetContentRegionAvail().x;
		ImGui::InvisibleButton("GradientCanvas", ImVec2(Width, 58.0f));
		const ImVec2 CanvasMin = ImGui::GetItemRectMin();
		const ImVec2 CanvasMax = ImGui::GetItemRectMax();
		const ImRect BarRect(CanvasMin, ImVec2(CanvasMax.x, CanvasMin.y + 30.0f));
		ImDrawList* DrawList = ImGui::GetWindowDrawList();

		const int SegmentCount = 96;
		for (int Segment = 0; Segment < SegmentCount; Segment++)
		{
			const float T0 = (float)Segment / (float)SegmentCount;
			const float T1 = (float)(Segment + 1) / (float)SegmentCount;
			const ImVec4 C0 = ToImVec4(Curve.Interpolate(T0));
			const ImVec4 C1 = ToImVec4(Curve.Interpolate(T1));
			const float X0 = ImLerp(BarRect.Min.x, BarRect.Max.x, T0);
			const float X1 = ImLerp(BarRect.Min.x, BarRect.Max.x, T1);
			DrawList->AddRectFilledMultiColor(
				ImVec2(X0, BarRect.Min.y),
				ImVec2(X1, BarRect.Max.y),
				ImGui::ColorConvertFloat4ToU32(C0),
				ImGui::ColorConvertFloat4ToU32(C1),
				ImGui::ColorConvertFloat4ToU32(C1),
				ImGui::ColorConvertFloat4ToU32(C0));
		}

		DrawList->AddRect(BarRect.Min, BarRect.Max, ImGui::GetColorU32(ImVec4(0.25f, 0.25f, 0.25f, 1.0f)), 6.0f, 0, 1.5f);

		const bool bCanvasHovered = ImGui::IsItemHovered();
		if (bCanvasHovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			GActiveCurveEditorId = EditorId;
			const float T = std::clamp((ImGui::GetIO().MousePos.x - BarRect.Min.x) / std::max(BarRect.GetWidth(), 1.0f), 0.0f, 1.0f);
			Curve.Points.emplace_back(T, Curve.Interpolate(T));
			Curve.Sort();
			State.SelectedIndex = FindClosestCurvePointIndex(Curve, T);
			Changed = true;
		}

		const ImVec2 MousePosition = ImGui::GetIO().MousePos;
		const int HoveredPoint = FindHoveredColorGradientStop(Curve, BarRect, MousePosition);
		if (HoveredPoint >= 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			GActiveCurveEditorId = EditorId;
			State.SelectedIndex = HoveredPoint;
			State.ActiveGradientStop = HoveredPoint;
		}
		else if (bCanvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			GActiveCurveEditorId = EditorId;
			State.ActiveGradientStop = -1;
		}

		if (State.ActiveGradientStop >= 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			const float T = std::clamp((MousePosition.x - BarRect.Min.x) / std::max(BarRect.GetWidth(), 1.0f), 0.0f, 1.0f);
			Curve.Points[State.ActiveGradientStop].Key = T;
			Curve.Sort();
			State.SelectedIndex = FindClosestCurvePointIndex(Curve, T);
			State.ActiveGradientStop = State.SelectedIndex;
			Changed = true;
		}
		else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			State.ActiveGradientStop = -1;
		}

		for (int PointIndex = 0; PointIndex < (int)Curve.Points.size(); PointIndex++)
		{
			DrawColorGradientStopHandle(DrawList, BarRect, ToImVec4(Curve.Points[PointIndex].Value), Curve.Points[PointIndex].Key, PointIndex == State.SelectedIndex);
		}

		if (State.SelectedIndex >= (int)Curve.Points.size())
			State.SelectedIndex = (int)Curve.Points.size() - 1;

		if (State.SelectedIndex >= 0 && State.SelectedIndex < (int)Curve.Points.size())
		{
			ImGui::Spacing();
			ImGui::TextDisabled("Selected stop %d", State.SelectedIndex);

			float SelectedKey = Curve.Points[State.SelectedIndex].Key;
			if (ImGui::SliderFloat("Stop Time", &SelectedKey, 0.0f, 1.0f, "%.3f"))
			{
				GActiveCurveEditorId = EditorId;
				Curve.Points[State.SelectedIndex].Key = SelectedKey;
				Curve.Sort();
				State.SelectedIndex = FindClosestCurvePointIndex(Curve, SelectedKey);
				Changed = true;
			}

			if constexpr (std::is_same_v<CurveValueType, Vector3>)
			{
				if (ImGui::ColorEdit3("Colour", &Curve.Points[State.SelectedIndex].Value.X, ImGuiColorEditFlags_Float))
				{
					GActiveCurveEditorId = EditorId;
					Changed = true;
				}
			}
			else
			{
				if (ImGui::ColorEdit4("Colour", &Curve.Points[State.SelectedIndex].Value.X, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar))
				{
					GActiveCurveEditorId = EditorId;
					Changed = true;
				}
			}
		}

		return Changed;
	}

	template <typename CurveType>
	static bool ImGui_EditInterpolationCurve(char* Object, const Reflection::Field& Field, int Depth)
	{
		using CurveValueType = std::decay_t<decltype(std::declval<CurveType>().Points[0].Value)>;
		(void)Depth;

		CurveType& Curve = *reinterpret_cast<CurveType*>(Object);
		const FieldUIMetadata Meta = ParseFieldMetadata(Field.Meta);

		ImGui::PushID((void*)Object);
		ImGui::PushID(Field.Name);
		const ImGuiID EditorId = ImGui::GetID("InterpolationCurveEditor");
		CurveEditorState& State = GCurveEditorStates[EditorId];
		if (!State.PlotView.bInitialised)
			FitCurvePlotView(Curve, State);
		ImGui::PushID((int)EditorId);

		bool Changed = false;
		if constexpr (std::is_same_v<CurveValueType, Vector3> || std::is_same_v<CurveValueType, Vector4>)
		{
			if (Meta.IsGradient)
				Changed = DrawGradientInterpolationCurveEditor(Curve, Field, State, EditorId);
			else
				Changed = DrawNumericInterpolationCurveEditor(Curve, Field, State, EditorId);
		}
		else
		{
			Changed = DrawNumericInterpolationCurveEditor(Curve, Field, State, EditorId);
		}

		ImGui::PopID();
		ImGui::PopID();
		ImGui::PopID();
		return Changed;
	}

	bool ImGui_EditVector2(char* Object, const Reflection::Field& Field, int Depth)
	{
		auto Meta = ParseFieldMetadata(Field.Meta);

		if (Meta.UseChannelColours)
			return DrawInlineFloatComponents(Field.Name, (float*)Object, 2, GVectorChannelLabels, true, true);

		return DrawInlineFloatComponents(Field.Name, (float*)Object, 2, GVectorChannelLabels, false, true);
	}

	bool ImGui_EditVector3(char* Object, const Reflection::Field& Field, int Depth)
	{
		auto Meta = ParseFieldMetadata(Field.Meta);

		if (Meta.IsColor)
		{
			ImGuiColorEditFlags Flags = ImGuiColorEditFlags_Float;
			if (Meta.IsHDR)
				Flags |= ImGuiColorEditFlags_HDR;

			return ImGui::ColorEdit3(Field.Name, (float*)Object, Flags);
		}

		if (Meta.UseChannelColours)
			return DrawInlineFloatComponents(Field.Name, (float*)Object, 3, GVectorChannelLabels, true, true);

		return DrawInlineFloatComponents(Field.Name, (float*)Object, 3, GVectorChannelLabels, false, true);
	}

	bool ImGui_EditVector4(char* Object, const Reflection::Field& Field, int Depth)
	{
		auto Meta = ParseFieldMetadata(Field.Meta);

		if (Meta.IsColor)
		{
			ImGuiColorEditFlags Flags = ImGuiColorEditFlags_Float;
			if (Meta.IsHDR)
				Flags |= ImGuiColorEditFlags_HDR;

			return ImGui::ColorEdit4(Field.Name, (float*)Object, Flags);
		}

		if (Meta.UseChannelColours)
			return DrawInlineFloatComponents(Field.Name, (float*)Object, 4, GVectorChannelLabels, true, true);

		return DrawInlineFloatComponents(Field.Name, (float*)Object, 4, GVectorChannelLabels, false, true);
	}

	bool ImGui_EditQuaternion(char* Object, const Reflection::Field& Field, int Depth)
	{
		Quaternion* Quat = (Quaternion*)Object;
		Vector3 Euler = Quat->Euler();
		bool Changed = DrawInlineFloatComponents(Field.Name, (float*)&Euler, 3, GVectorChannelLabels, true, true);
		if (Changed)
		{
			*Quat = Quaternion(Euler);
		}
		return Changed;
	}

	CREFLECT_STRUCT_CUSTOM_UI(Vector2, ImGui_EditVector2);
	CREFLECT_STRUCT_CUSTOM_UI(Vector3, ImGui_EditVector3);
	CREFLECT_STRUCT_CUSTOM_UI(Vector4, ImGui_EditVector4);
	CREFLECT_STRUCT_CUSTOM_UI(Quaternion, ImGui_EditQuaternion);
	CREFLECT_STRUCT_CUSTOM_UI(InterpolationCurveFloat1, ImGui_EditInterpolationCurve<InterpolationCurveFloat1>);
	CREFLECT_STRUCT_CUSTOM_UI(InterpolationCurveFloat2, ImGui_EditInterpolationCurve<InterpolationCurveFloat2>);
	CREFLECT_STRUCT_CUSTOM_UI(InterpolationCurveFloat3, ImGui_EditInterpolationCurve<InterpolationCurveFloat3>);
	CREFLECT_STRUCT_CUSTOM_UI(InterpolationCurveFloat4, ImGui_EditInterpolationCurve<InterpolationCurveFloat4>);

	// Custom UI specialisations for reflected types
	///////////////////////////////////////////////////////////////////////////////////////////////


	void ApplyDarkTheme()
	{
		ImGuiStyle& Style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&Style);
		Style.FrameRounding = 3.0f;
		Style.WindowRounding = 0.0f;
		Style.ScrollbarRounding = 3.0f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.75f, 0.75f, 0.75f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
	}

	void FlagButton(const char* name, bool& enabled, const char* tooltip)
	{
		if (ImGui::Button(name, enabled))
			enabled = !enabled;

		if (tooltip != NULL)
			ShowTooltipDelayed(CommonUISettings.TooltipDelay, tooltip);
	}

	bool ToolButton(const char* label, int* v, int v_button, const ImVec2& size, const char* tooltip)
	{
		const bool pressed = ImGui::Button(label, *v == v_button, false, size);
		if (pressed)
			*v = v_button;

		if (tooltip != NULL)
			ShowTooltipDelayed(CommonUISettings.TooltipDelay, tooltip);

		return pressed;
	}

	void ToggleButton(const char* label, bool* v, const char* tooltip)
	{
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float height = ImGui::GetFrameHeight();
		float width = height * 1.55f;
		float radius = height * 0.50f;

		ImGui::InvisibleButton(label, ImVec2(width, height));
		if (ImGui::IsItemClicked())
			*v = !*v;

		if (tooltip != NULL)
			ShowTooltipDelayed(CommonUISettings.TooltipDelay, tooltip);

		float t = *v ? 1.0f : 0.0f;

		ImGuiContext& g = *GImGui;
		float ANIM_SPEED = 0.08f;
		if (g.LastActiveId == g.CurrentWindow->GetID(label))// && g.LastActiveIdTimer < ANIM_SPEED)
		{
			float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
			t = *v ? (t_anim) : (1.0f - t_anim);
		}

		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
		else
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
	}

	void ShowTooltipDelayed(float delay, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		if (ImGui::IsItemHovered() && GImGui->HoveredIdTimer > delay)
		{
			ImGui::SetTooltipV(fmt, args);
		}

		va_end(args);
	}

	const char* GetFileIcon(const std::string& ext)
	{
		std::string e = str_tolower(ext);

		if (Filesystem::IsImage(ext)) return ICON_FA_FILE_IMAGE;
		if (e == "wav" || e == "mp3" || e == "ogg") return ICON_FA_MUSIC;
		if (e == "json" || e == "glsl" || e == "hlsl" || e == "csl") return ICON_FA_CODE;
		if (e == "hdr" || e == "exr") return ICON_FA_FILE_IMAGE;
		if (e == "scene") return ICON_FA_STRIKETHROUGH;
		if (e == "lig") return LIGHT_ICON;
		if (e == "mat") return MATERIAL_ICON;
		if (e == "par") return PARTICLES_ICON;
		if (e == "cmf" || e == "obj" || e == "dae" || e == "fbx") return MESH_ICON;
		if (e == "ttf") return ICON_FA_FONT;

		return ICON_FA_FILE;
	}

	struct InternalModalWindow
	{
		std::string Name;
		std::function<bool()> DrawCallback;
	};

	static std::vector<InternalModalWindow> ModalWindowsList;

	void ShowModalWindow(const char* Name, std::function<bool()> DrawCallback)
	{
		ModalWindowsList.push_back(InternalModalWindow
		{
			.Name = Name,
			.DrawCallback = DrawCallback,
		});
	}

	void TickAllModalWindows()
	{
		for (int i = 0; i < (int)ModalWindowsList.size(); i++)
		{
			InternalModalWindow& Window = ModalWindowsList[i];
			bool Open = true;
			bool WasClosed = false;

			ImVec2 Size(500, 250);

			ImGui::OpenPopup(Window.Name.c_str());
			ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(Size.x, Size.y));
			if (ImGui::BeginPopupModal(Window.Name.c_str(), &Open, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
			{
				if (Window.DrawCallback())
				{
					ModalWindowsList.erase(ModalWindowsList.begin() + i--);
					WasClosed = true;
				}

				ImGui::EndPopup();
			}

			if (!Open && !WasClosed)
			{
				ModalWindowsList.erase(ModalWindowsList.begin() + i--);
			}
		}
	}

	void ShowMessageBox(const char* Name, const char* Text, MessageBoxCallbacks Callbacks)
	{
		std::string TextStr = Text;

		ShowModalWindow(Name, [TextStr, Callbacks]() -> bool
		{
			ImVec2 Size = ImGui::GetContentRegionAvail();
			if (ImGui::BeginChild("TextField##MessageBox", ImVec2(Size.x, Size.y - 30)))
			{
				ImGui::TextWrapped("%s", TextStr.c_str());
			}
			ImGui::EndChild();

			bool Result = false;

			if (ImGui::BeginChild("Buttons##MessageBox"))
			{
				if (ImGui::Button("Cancel"))
				{
					if (Callbacks.OnCancel)
						Callbacks.OnCancel();
					Result = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("No"))
				{
					if (Callbacks.OnNo)
						Callbacks.OnNo();
					Result = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Yes"))
				{
					if (Callbacks.OnYes)
						Callbacks.OnYes();
					Result = true;
				}
			}
			ImGui::EndChild();

			return Result;
		});
	}



	///////////////////////////////////////////////////////////////////////////////////////////////
	// Editing of reflected objects

	bool Reflection_EditObjectField(char* Object, const Reflection::Field& Field, int Depth)
	{
		char* FieldData = Object + Field.Offset;

		auto Metadata = ParseFieldMetadata(Field.Meta);

		switch (Field.Type)
		{
		case Reflection::FieldType::Bool:
			if (Metadata.NoEdit)
			{
				ImGui::LabelText(Field.Name, "%s", *(bool*)FieldData ? "true" : "false");
				return false;
			}
			return ImGui::Checkbox(Field.Name, (bool*)FieldData);
			break;
		case Reflection::FieldType::Int:
			if (Metadata.NoEdit)
			{
				ImGui::LabelText(Field.Name, "%i", *(int*)FieldData);
				return false;
			}
			if (Metadata.CommitOnEnter)
			{
				ImGui::InputInt(Field.Name, (int*)FieldData, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue);
				return ImGui::IsItemDeactivatedAfterEdit();
			}
			return ImGui::DragInt(Field.Name, (int*)FieldData, 1.0f);
			break;
		case Reflection::FieldType::Float:
		{
			if (Metadata.NoEdit)
			{
				ImGui::LabelText(Field.Name, "%.3f", *(float*)FieldData);
				return false;
			}

			if (Metadata.HasSliderLimits)
			{
				return ImGui::SliderFloat(Field.Name, (float*)FieldData, Metadata.SliderMin, Metadata.SliderMax);
			}

			return ImGui::DragFloat(Field.Name, (float*)FieldData, 0.01f, 0.0f, 0.0f, "%.3f");

			break;
		}
		case Reflection::FieldType::String:
		{
			if (strstr(Field.Meta, "Picker"))
			{
				ImGui::LabelText(Field.Name, "%s", ((std::string*)FieldData)->c_str());
				ImGui::SameLine();
				if (ImGui::Button("..."))
				{
					char* path = nullptr;
					if (NFD_OpenDialog("gltf,clvl", nullptr, &path) == NFD_OKAY)
					{
						((std::string*)FieldData)->assign(path);
						return true;
					}

					return false;
				}
			}
			else if (Metadata.NoEdit)
			{
				ImGui::LabelText(Field.Name, "%s", ((std::string*)FieldData)->c_str());
			}
			else
			{
				return ImGui::InputText(Field.Name, (std::string*)FieldData);
			}
		}
		break;

		case Reflection::FieldType::Blob:
		{
			Blob* DataBlob = reinterpret_cast<Blob*>(FieldData);
			double HumanisedSize = 0.0;
			const char* Units = HumanizeBytes(DataBlob->Size(), HumanisedSize);
			ImGui::LabelText(Field.Name, "%.2f %s (%llu bytes)", HumanisedSize, Units, (unsigned long long)DataBlob->Size());
			return false;
		}
		break;

		case Reflection::FieldType::Array:
		{
			std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
			Reflection::ArrayData* Array = Field.Array;
			const bool bAllowArrayResize = !Metadata.NoArrayResize;

			u32 ElementSize = Array->ElementField.Size;
			u32 NumElements = (u32)ArrayData->size() / ElementSize;

			bool Result = false;
			if (ImGui::TreeNodeEx(Field.Name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding))
			{
				ImDrawList* DrawList = ImGui::GetWindowDrawList();
				const float ChildIndentation = 5.0f;
				ImGui::Indent(ChildIndentation);
				const ImVec2 ChildRegionStart = ImGui::GetCursorScreenPos();
				const float GuideX = ChildRegionStart.x - ImGui::GetTreeNodeToLabelSpacing() * 0.5f;

				if (NumElements == 0)
				{
					ImGui::TextDisabled("Empty array");
				}

				i32 ElementToRemove = -1;

				u32 Offset = 0;
				for (u32 i = 0; i < NumElements; i++)
				{
					ImGui::PushID(i);

					char* ElementData = ArrayData->data() + Offset;
					const Reflection::Field& ElementField = Array->ElementField;
					bool bElementChanged = false;

					if (ElementField.Type == Reflection::FieldType::Struct && ElementField.Struct && !ElementField.Struct->CustomUI)
					{
						std::string HeaderLabel = GetArrayElementHeaderLabel(ElementField.Struct, ElementData, i);
						bool bOpen = ImGui::TreeNodeEx(HeaderLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_FramePadding);
						if (bAllowArrayResize)
						{
							ImGui::SameLine();
							if (ImGui::Button(ICON_FA_TRASH_ALT " Remove"))
							{
								ElementToRemove = i;
							}
						}

						if (bOpen)
						{
							const float TreeContentIndentation = ImGui::GetTreeNodeToLabelSpacing();
							ImGui::Indent(TreeContentIndentation);
							for (const Reflection::Field& StructField : ElementField.Struct->Fields)
							{
								if (ParseFieldMetadata(StructField.Meta).Hidden)
									continue;
								bElementChanged |= Reflection_EditObjectField(ElementData, StructField, Depth + 1);
							}
							ImGui::Unindent(TreeContentIndentation);
						}
					}
					else
					{
						bElementChanged |= Reflection_EditObjectField(ElementData, ElementField, Depth + 1);
						if (bAllowArrayResize)
						{
							ImGui::SameLine();
							if (ImGui::Button(ICON_FA_TRASH_ALT " Remove"))
							{
								ElementToRemove = i;
							}
						}
					}
					Result |= bElementChanged;

					ImGui::PopID();

					Offset += ElementSize;
				}
				if (bAllowArrayResize && ImGui::Button(ICON_FA_PLUS " Add Element"))
				{
					Array->NewElement(FieldData);
					Result = true;
				}

				if (ElementToRemove > -1)
				{
					Array->DeleteElement(FieldData, ElementToRemove);
					Result = true;
				}

				const ImVec2 ChildRegionEnd = ImGui::GetItemRectMax();
				if (ChildRegionEnd.y > ChildRegionStart.y)
				{
					ImVec4 GuideColour = ImGui::GetStyleColorVec4(ImGuiCol_Separator);
					GuideColour.w *= 0.65f;
					DrawList->AddLine(
						ImVec2(GuideX, ChildRegionStart.y),
						ImVec2(GuideX, ChildRegionEnd.y),
						ImGui::GetColorU32(GuideColour),
						1.0f);
				}

				ImGui::Unindent(ChildIndentation);
				ImGui::TreePop();
			}
			return Result;
		}
		break;

		case Reflection::FieldType::Enum:
		{
			int EnumValue = *(int*)FieldData;

			const Reflection::Enum* Enum = Field.Enum;
			const Reflection::EnumField* EnumField = Enum->FindFieldByValue(EnumValue);
			int Idx = EnumField->Index;

			bool Result = ImGui::Combo(Field.Name, &Idx, [](void* data, int idx, const char** out_text) -> bool
				{
					*out_text = ((Reflection::Enum*)data)->Fields[idx].Name;
					return true;
				}, (void*)Enum, Enum->Fields.size());

			*((int*)(FieldData)) = Enum->Fields[Idx].Value;

			return Result;
		}
		break;


		case Reflection::FieldType::Struct:
		{
			const Reflection::Struct* Struct = Field.Struct;
			float Indentation = Depth * 5.0f;
			bool Result = false;

			if (Struct->CustomUI)
			{
				Result = Struct->CustomUI(FieldData, Field, Depth + 1);
			}
			else
			{
				if (ImGui::CollapsingHeader(Field.Name))
				{
					for (const auto& SField : Struct->Fields)
					{
						ImGui::Indent(Indentation);
						ImGui::PushID(SField.Name);
						Result |= Reflection_EditObjectField(FieldData, SField, Depth + 1);
						ImGui::PopID();
						ImGui::Unindent(Indentation);
					}
				}
			}

			return Result;
		}
		break;

		case Reflection::FieldType::AssetRef:
		{
			struct AssetRefBase
			{
				std::string Path;
				void* Asset = nullptr;
			};

			AssetRefBase* AssetRef = reinterpret_cast<AssetRefBase*>(FieldData);
			char ButtonBuf[512]{};
			snprintf(ButtonBuf, 512, "...##%s", Field.Name);
			char ClearButtonBuf[512]{};
			snprintf(ClearButtonBuf, 512, "X##clear_%s", Field.Name);

			ImGui::LabelText(Field.Name, "%s", AssetRef->Path.c_str());
			ImGui::SameLine();
			if (ImGui::Button(ButtonBuf))
			{
				char* path = nullptr;

				const char* AssetExts = "";
				const Reflection::Struct* AssetType = Reflection::FindStructByGuid(Field.Typeguid);

				if (AssetSystem::Get().AssetExtensions.contains(AssetType))
				{
					AssetExts = AssetSystem::Get().AssetExtensions[AssetType];
				}

				if (NFD_OpenDialog(AssetExts, nullptr, &path) == NFD_OKAY)
				{
					if (!AssetSystem::Get().IsPathInBakedFolder(path))
					{
						const auto& AssetBasePath = AssetSystem::Get().DataPath;

						char ErrorBuf[4096]{ 0 };
						snprintf(ErrorBuf, 4096, "Cannot choose asset (%s) - any referenced asset has to be under project's Data folder (%s)", path, AssetBasePath.c_str());

						Editor::ShowMessageBox("Asset Reference Error", ErrorBuf, {});
						Log::Error(ErrorBuf);
						return false;
					}

					AssetRef->Path = AssetSystem::Get().MakePathRelativeToBakedFolder(path);
					return true;
				}

				return false;
			}
			ImGui::SameLine();
			if (ImGui::Button(ClearButtonBuf))
			{
				struct AssetRefMutableBase
				{
					std::string Path;
					void* Asset = nullptr;
					void Unload()
					{
						AssetSystem::Get().UnloadAssetRaw(Asset);
						Asset = nullptr;
					}
				};

				AssetRefMutableBase* MutableRef = reinterpret_cast<AssetRefMutableBase*>(FieldData);
				MutableRef->Unload();
				MutableRef->Path.clear();
				return true;
			}

			const Reflection::Struct* AssetType = Reflection::FindStructByGuid(Field.Typeguid);
			if (AssetType == Reflection::FindStruct<Texture2>())
			{
				Columbus::AssetRef<Texture2> PreviewRef;
				Texture2* PreviewTexture = static_cast<Texture2*>(AssetRef->Asset);

				if (!PreviewTexture && !AssetRef->Path.empty() && AssetSystem::Get().HasPath(AssetRef->Path))
				{
					PreviewRef = AssetSystem::Get().GetRefByPath<Texture2>(AssetRef->Path);
					PreviewTexture = PreviewRef.Asset;
				}

				if (PreviewTexture)
				{
					const TextureDesc2& Desc = PreviewTexture->GetDesc();
					const std::string AssetName = AssetRef->Path.empty()
						? std::string("Texture")
						: std::filesystem::path(AssetRef->Path).filename().string();
					const float FitZoomX = 96.0f / (float)Math::Max(1u, Desc.Width);
					const float FitZoomY = 96.0f / (float)Math::Max(1u, Desc.Height);
					DebugUI::TextureWidgetSettings PreviewSettings;
					PreviewSettings.ShowCheckerboard = false;
					PreviewSettings.Zoom = std::min(1.0f, std::min(FitZoomX, FitZoomY));

					ImGui::BeginChild((std::string("##TexturePreview") + Field.Name).c_str(), ImVec2(0.0f, 124.0f), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
					DebugUI::TextureWidget(PreviewTexture, Vector2(96.0f, 96.0f), PreviewSettings);
					ImGui::SameLine();
					ImGui::BeginGroup();
					ImGui::TextUnformatted(AssetName.c_str());
					ImGui::TextDisabled("%u x %u", Desc.Width, Desc.Height);
					ImGui::TextDisabled("%s", AssetRef->Path.c_str());
					ImGui::EndGroup();
					ImGui::EndChild();
				}
			}
			break;
		}

		case Reflection::FieldType::ThingRef:
		{
			ThingRef<AThing>* Ref = reinterpret_cast<ThingRef<AThing>*>(FieldData);

			// Get all things in the world
			std::vector<AThing*> things;
			if (GCurrentProject && GCurrentProject->World)
			{
				for (AThing* thing : GCurrentProject->World->AllThings)
				{
					if (Reflection::HasParentType(thing->GetTypeVirtual(), Reflection::FindStructByGuid(Field.Typeguid)))
					{
						things.push_back(thing);
					}
				}
			}

			// Build display names
			std::vector<std::string> thingNames;
			int currentIdx = -1;
			for (size_t i = 0; i < things.size(); ++i)
			{
				const auto& thing = things[i];
				thingNames.push_back(thing->Name + " (" + std::to_string((u64)thing->Guid) + ")");
				if (Ref->Thing == thing)
					currentIdx = static_cast<int>(i);
			}

			// Add "None" option
			thingNames.insert(thingNames.begin(), "None");
			if (currentIdx == -1)
				currentIdx = 0;
			else
				currentIdx += 1;

			bool changed = false;
			if (ImGui::Combo(Field.Name, &currentIdx,
				[](void* data, int idx, const char** out_text) -> bool {
					auto& names = *static_cast<std::vector<std::string>*>(data);
					*out_text = names[idx].c_str();
					return true;
				}, &thingNames, static_cast<int>(thingNames.size())))
			{
				if (currentIdx == 0)
				{
					Ref->Thing = nullptr;
					Ref->Guid = 0;
				}
				else
				{
					Ref->Thing = things[currentIdx - 1];
					Ref->Guid = Ref->Thing->Guid;
				}
				changed = true;
			}
			return changed;
		}

		default:
			ImGui::LabelText("Unsupported Type", "%s %s", Field.Typename, Field.Name);
			break;
		}

		return false;
	}

	bool Reflection_EditStructContents(char* Object, const Reflection::Struct* Struct, int Depth)
	{
		bool AnyFieldChanged = false;

		for (const auto& Field : Struct->Fields)
		{
			if (ParseFieldMetadata(Field.Meta).Hidden)
				continue;
			AnyFieldChanged |= Reflection_EditObjectField(Object, Field, Depth);
		}

		if (AnyFieldChanged && Struct->ChangeNotify)
		{
			Struct->ChangeNotify(Object);
		}

		return AnyFieldChanged;
	}

	bool Reflection_EditStruct(char* Object, const Reflection::Struct* Struct)
	{
		ImGui::TextDisabled("%s", Struct->Name);
		ImGui::SameLine();
		ImGui::TextDisabled("v%i", Struct->Version);
		ImGui::SameLine();
		ImGui::TextDisabled("%s", Struct->Guid);
		ImGui::Separator();

		return Reflection_EditStructContents(Object, Struct);
	}

	// Editing of reflected objects
	///////////////////////////////////////////////////////////////////////////////////////////////
}

namespace ImGui
{
	void SetNextWindowPosCenter(ImGuiCond cond)
	{
		auto DisplaySize = ImGui::GetIO().DisplaySize;
		auto Center = ImVec2(DisplaySize.x / 2, DisplaySize.y / 2);
		ImGui::SetNextWindowPos(Center, cond, ImVec2(0.5f, 0.5f));
	}

	bool TreeNodeSized(const char* label, ImVec2 size, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		float LineSizeY = window->DC.CurrLineSize.y;
		float LineBaseOffset = window->DC.CurrLineTextBaseOffset;

		ImVec2 Size = size;

		if (Size.x == 0) Size.x = Columbus::Editor::CommonUISettings.TreeNodeSize.x;
		if (Size.y == 0) Size.y = Columbus::Editor::CommonUISettings.TreeNodeSize.y;

		if (Size.x < 0) Size.x += GetContentRegionAvail().x;
		if (Size.y < 0) Size.y += GetContentRegionAvail().y;

		bool Open = false;
		if (ImGui::BeginChild("TreeNode", Size))
		{
			window = GetCurrentWindow();
			window->DC.CurrLineSize.y = LineSizeY;
			window->DC.CurrLineTextBaseOffset = LineBaseOffset;

			Open = ImGui::TreeNodeEx(label, flags);
		}
		ImGui::EndChild();

		return Open;
	}

	bool Button(const char* label, bool activated, bool hover_col_change, const ImVec2& size)
	{
		auto active = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
		auto passive = ImGui::GetStyle().Colors[ImGuiCol_Button];
		auto current = activated ? active : passive;

		if (!hover_col_change)
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, current);

		ImGui::PushStyleColor(ImGuiCol_Button, current);
		bool result = ImGui::Button(label, size);
		ImGui::PopStyleColor();

		if (!hover_col_change)
			ImGui::PopStyleColor();

		return result;
	}

	/*void Image(Columbus::Texture* texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		switch (Columbus::gDevice->GetCurrentAPI())
		{
			case Columbus::GraphicsAPI::None: break;
			case Columbus::GraphicsAPI::OpenGL:
			{
				ImTextureID id = nullptr;
				if (texture)
					id = (ImTextureID)static_cast<Columbus::TextureOpenGL*>(texture)->GetID();

				Image(id, size, uv0, uv1, tint_col, border_col);
				break;
			}
		}
	}*/
}
