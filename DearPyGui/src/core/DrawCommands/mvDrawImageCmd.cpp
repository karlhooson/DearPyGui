#include "mvDrawImageCmd.h"
#include "Registries/mvTextureStorage.h"
#include "PythonUtilities/mvPythonTranslator.h"

namespace Marvel {

	mvDrawImageCmd::mvDrawImageCmd(std::string file, const mvVec2& pmin, const mvVec2& pmax, const mvVec2& uv_min,
		const mvVec2& uv_max, const mvColor& color)
		:
		m_file(std::move(file)),
		m_pmax(pmax),
		m_pmin(pmin),
		m_uv_min(uv_min),
		m_uv_max(uv_max),
		m_color(color)
	{
	}

	mvDrawImageCmd::~mvDrawImageCmd()
	{
		mvTextureStorage::DecrementTexture(m_file);
	}

	void mvDrawImageCmd::draw(ImDrawList* drawlist, float x, float y)
	{

		if (mvTextureStorage::GetTexture(m_file))
		{
			if (mvTextureStorage::GetTexture(m_file)->texture != m_texture)
				m_texture = nullptr;
		}
		else
			m_texture = nullptr;

		if (m_texture == nullptr && !m_file.empty())
		{

			mvTextureStorage::AddTexture(m_file);
			mvTexture* texture = mvTextureStorage::GetTexture(m_file);
			if (texture == nullptr)
			{
				PyErr_Format(PyExc_Exception,
					"Image %s could not be found for draw_image. Check the path to the image "
					"you provided.", m_file.c_str());
				PyErr_Print();
				m_file = "";
				return;
			}

			m_texture = texture->texture;

			if (m_width == 0) m_width = (int)((float)texture->width * (m_uv_max.x - m_uv_min.x));
			if (m_height == 0) m_height = (int)((float)texture->height * (m_uv_max.y - m_uv_min.y));

		}

		mvVec2 start = {x, y};

		if (m_texture)
			drawlist->AddImage(m_texture, m_pmin + start, m_pmax + start, m_uv_min, m_uv_max, m_color);
	}

	void mvDrawImageCmd::setConfigDict(PyObject* dict)
	{
		if (dict == nullptr)
			return;
		mvGlobalIntepreterLock gil;

		if (PyObject* item = PyDict_GetItemString(dict, "pmax")) m_pmax = ToVec2(item);
		if (PyObject* item = PyDict_GetItemString(dict, "pmin")) m_pmin = ToVec2(item);
		if (PyObject* item = PyDict_GetItemString(dict, "uv_min")) m_uv_min = ToVec2(item);
		if (PyObject* item = PyDict_GetItemString(dict, "uv_min")) m_uv_max = ToVec2(item);
		if (PyObject* item = PyDict_GetItemString(dict, "color")) m_color = ToColor(item);

		if (PyObject* item = PyDict_GetItemString(dict, "file"))
		{
			if (m_file != ToString(item))
			{
				mvTextureStorage::DecrementTexture(m_file);
				m_texture = nullptr;
			}
			m_file = ToString(item);

		}

	}

	void mvDrawImageCmd::getConfigDict(PyObject* dict)
	{
		if (dict == nullptr)
			return;
		mvGlobalIntepreterLock gil;
		PyDict_SetItemString(dict, "pmax", ToPyPair(m_pmax.x, m_pmax.y));
		PyDict_SetItemString(dict, "pmin", ToPyPair(m_pmin.x, m_pmin.y));
		PyDict_SetItemString(dict, "uv_min", ToPyPair(m_uv_min.x, m_uv_min.y));
		PyDict_SetItemString(dict, "uv_max", ToPyPair(m_uv_max.x, m_uv_max.y));
		PyDict_SetItemString(dict, "color", ToPyColor(m_color));
		PyDict_SetItemString(dict, "file", ToPyString(m_file));
	}

}