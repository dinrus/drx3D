#pragma once

#include <drx3D/Maths/Color.h>
#include <drx3D/Maths/Vector2.h>
#include <drx3D/Uis/Drivers/UiDriver.h>
#include <drx3D/Geometry/Model.h>
#include <drx3D/Geometry/Vertex2d.h>
#include <drx3D/Graphics/Descriptors/DescriptorsHandler.h>
#include <drx3D/Graphics/Buffers/UniformHandler.h>
#include <drx3D/Graphics/Pipelines/PipelineGraphics.h>
#include <drx3D/Uis/UiObject.h>
#include <drx3D/Fonts/FontType.h>

namespace drx3d {
class DRX3D_EXPORT VertexText {
public:
	VertexText() = default;
	VertexText(const Vector2f &position, const Vector3f &uv) :
		position(position),
		uv(uv) {
	}

	static Shader::VertexInput GetVertexInput(uint32_t baseBinding = 0) {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
			{baseBinding, sizeof(VertexText), VK_VERTEX_INPUT_RATE_VERTEX}
		};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			{0, baseBinding, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexText, position)},
			{1, baseBinding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexText, uv)}
		};
		return {bindingDescriptions, attributeDescriptions};
	}

	bool operator==(const VertexText &rhs) const {
		return position == rhs.position && uv == rhs.uv;
	}

	bool operator!=(const VertexText &rhs) const {
		return !operator==(rhs);
	}

	Vector2f position;
	Vector3f uv;
};

/**
 * @brief Class that represents a text in a GUI.
 */
class DRX3D_EXPORT Text : public UiObject {
	friend class FontType;
public:
	/**
	 * @brief A enum that represents how the text will be justified.
	 */
	enum class Justify {
		Left, Centre, Right, Fully
	};

	Text() = default;

	void UpdateObject() override;

	bool CmdRender(const CommandBuffer &commandBuffer, const PipelineGraphics &pipeline);

	/**
	 * Gets the text model, which contains all the vertex data for the quads on which the text will be rendered.
	 * @return The model of the text.
	 */
	const Model *GetModel() const { return model.get(); }

	/**
	 * Gets the font size.
	 * @return The font size.
	 */
	float GetFontSize() const { return fontSize; }

	/**
	 * Sets the font size.
	 * @param fontSize The new font size,
	 */
	void SetFontSize(float fontSize);

	/**
	 * Gets the number of lines in this text.
	 * @return The number of lines.
	 */
	uint32_t GetNumberLines() const { return numberLines; }

	/**
	 * Gets the string of text represented.
	 * @return The string of text.
	 */
	const STxt &GetString() const { return string; }

	/**
	 * Changed the current string in this text.
	 * @param string The new text,
	 */
	void SetString(const STxt &string);

	/**
	 * Gets how the text should justify.
	 * @return How the text should justify.
	 */
	Justify GetJustify() const { return justify; }

	/**
	 * Sets how the text should justify.
	 * @param justify How the text will justify.
	 */
	void SetJustify(Justify justify);

	/**
	 * Gets the kerning (type character spacing multiplier) of this text.
	 * @return The type kerning.
	 */
	float GetKerning() const { return kerning; }

	/**
	 * Sets the kerning (type character spacing multiplier) of this text.
	 * @param kerning The new kerning.
	 */
	void SetKerning(float kerning);

	/**
	 * Gets the leading (vertical line spacing multiplier) of this text.
	 * @return The line leading.
	 */
	float GetLeading() const { return leading; }

	/**
	 * Sets the leading (vertical line spacing multiplier) of this text.
	 * @param leading The new leading.
	 */
	void SetLeading(float leading);

	/**
	 * Gets the font used by this text.
	 * @return The font used by this text.
	 */
	const std::shared_ptr<FontType> &GetFontType() const { return fontType; }

	/**
	 * Sets the font used by this text.
	 * @param fontType The font type to be used in this text.
	 */
	void SetFontType(const std::shared_ptr<FontType> &fontType);

	/**
	 * Gets the color of the text.
	 * @return The color of the text.
	 */
	const Color &GetTextColor() const { return textColor; }

	/**
	 * Sets the color of the text.
	 * @param textColor The new color of the text.
	 */
	void SetTextColor(const Color &textColor) { this->textColor = textColor; }

	/**
	 * Gets if the text has been loaded to a model.
	 * @return If the text has been loaded to a model.
	 */
	bool IsLoaded() const;

private:
	/**
	 * During the loading of a text this represents one word in the text.
	 */
	class Word {
	public:
		/**
		 * Creates a new text word.
		 */
		Word() = default;

		/**
		 * Adds a glyph to the end of the current word and increases the screen-space width of the word.
		 * @param glyph The glyph to be added.
		 * @param kerning The glyph kerning.
		 */
		void AddCharacter(const FontType::Glyph &glyph, float kerning) {
			glyphs.emplace_back(glyph);
			width += kerning + glyph.advance;
		}

		std::vector<FontType::Glyph> glyphs;
		float width = 0.0f;
	};

	/**
	 * Represents a line of text during the loading of a text.
	 */
	class Line {
	public:
		/**
		 * Creates a new text line.
		 * @param spaceWidth The screen-space width of a space character.
		 * @param maxLength The screen-space maximum length of a line.
		 */
		Line(float spaceWidth, float maxLength) :
			maxLength(maxLength),
			spaceSize(spaceWidth) {
		}

		/**
		 * Attempt to add a word to the line. If the line can fit the word in without reaching the maximum line length then the word is added and the line length increased.
		 * @param word The word to try to add.
		 * @return {@code true} if the word has successfully been added to the line.
		 */
		bool AddWord(const Word &word) {
			auto additionalLength = word.width;
			additionalLength += !words.empty() ? spaceSize : 0.0f;

			if (currentLineLength + additionalLength <= maxLength) {
				words.emplace_back(word);
				currentWordsLength += word.width;
				currentLineLength += additionalLength;
				return true;
			}

			return false;
		}

		float maxLength;
		float spaceSize;

		std::vector<Word> words;
		float currentWordsLength = 0.0f;
		float currentLineLength = 0.0f;
	};

	/**
	 * Takes in an unloaded text and calculate all of the vertices for the quads on which this text will be rendered.
	 * The vertex positions and texture coords and calculated based on the information from the font file.
	 * Then takes the information about the vertices of all the quads and stores it in a model.
	 */
	void LoadText();
	std::vector<Line> CreateStructure() const;
	void CompleteStructure(std::vector<Line> &lines, Line &currentLine, const Word &currentWord, float maxLength) const;
	std::vector<VertexText> CreateQuad(const std::vector<Line> &lines) const;
	static void AddVerticesForGlyph(float cursorX, float cursorY, const FontType::Glyph &glyph, std::vector<VertexText> &vertices);
	static void AddVertex(float vx, float vy, float tx, float ty, std::vector<VertexText> &vertices);

	DescriptorsHandler descriptorSet;
	UniformHandler uniformObject;

	std::unique_ptr<Model> model;
	uint32_t numberLines = 0;
	Vector2i lastSize;

	float fontSize = 12;
	STxt string;
	Justify justify = Justify::Left;
	bool dirty = true;

	std::shared_ptr<FontType> fontType;
	float kerning = 0.0f;
	float leading = 0.0f;

	Color textColor = Color::Black;
};
}
