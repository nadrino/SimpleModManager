//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include <GlobalObjects.h>
#include "TabModPlugins.h"

#include <PopupLoading.h>
#include "Logger.h"
#include "GenericToolbox.h"

#include <switch.h>

#include <thread>
#include <future>


LoggerInit([]{
  Logger::setUserHeaderStr("[tab_mod_plugins]");
});


std::map<std::string, brls::Image *> cached_thumbs;
TabModPlugins::TabModPlugins()
{

	this->frameCounter = -1;

	// Setup the list
	auto plugin_nros_list = GenericToolbox::getListOfEntriesInFolder(
      GlobalObjects::getModBrowser().get_current_directory() + "/.plugins");
	plugin_nros_list.erase(std::remove_if(plugin_nros_list.begin(), plugin_nros_list.end(), [this](std::string &x) {
							   return get_extension(x) != ".smm"; // put your condition here
						   }),
						   plugin_nros_list.end());
	for (int i_nro = 0; i_nro < int(plugin_nros_list.size()); i_nro++)
	{
		std::string selected_plugin = remove_extension(plugin_nros_list[i_nro]);
		std::string selected_plugin_path =
        GlobalObjects::getModBrowser().get_current_directory() + "/.plugins/" + plugin_nros_list[i_nro];
		std::string selected_plugin_author;
		std::string selected_plugin_version;
		LogDebug("Adding plugin: %s", selected_plugin.c_str());
		FILE *file = fopen(selected_plugin_path.c_str(), "rb");
		if (file)
		{
			char name[513];
			char author[257];
			char version[17];

			fseek(file, sizeof(NroStart), SEEK_SET);
			NroHeader header;
			fread(&header, sizeof(header), 1, file);
			fseek(file, header.size, SEEK_SET);
			NroAssetHeader asset_header;
			fread(&asset_header, sizeof(asset_header), 1, file);

			NacpStruct *nacp = (NacpStruct *)malloc(sizeof(NacpStruct));
			if (nacp != NULL)
			{
				fseek(file, header.size + asset_header.nacp.offset, SEEK_SET);
				fread(nacp, sizeof(NacpStruct), 1, file);

				NacpLanguageEntry *langentry = NULL;
				Result rc = nacpGetLanguageEntry(nacp, &langentry);
				if (R_SUCCEEDED(rc) && langentry != NULL)
				{
					strncpy(name, langentry->name, sizeof(name) - 1);
					strncpy(author, langentry->author, sizeof(author) - 1);
				}
				strncpy(version, nacp->display_version, sizeof(version) - 1);

				free(nacp);
				nacp = NULL;
			}

			selected_plugin_author = author;
			selected_plugin_version = version;

			fclose(file);
		}
		auto *item = new brls::ListItem(selected_plugin, "", selected_plugin_author);
		item->setValue(selected_plugin_version);
		item->getClickEvent()->subscribe([this, selected_plugin, selected_plugin_path](View *view) {
			auto *dialog = new brls::Dialog("Do you want to start \"" + selected_plugin + "\" ?");

			dialog->addButton("Yes", [selected_plugin_path, dialog](brls::View *view) {
				envSetNextLoad(selected_plugin_path.c_str(), selected_plugin_path.c_str());
				brls::Application::quit();
				dialog->close();
			});
			dialog->addButton("No", [dialog](brls::View *view) {
				dialog->close();
			});

			dialog->setCancelable(true);
			dialog->open();

			return true;
		});
		item->updateActionHint(brls::Key::A, "Start");
		item->setValueActiveColor(nvgRGB(80, 128, 80));
		brls::Image *icon = load_image_cache(selected_plugin_path);
		item->setThumbnail(icon);

		this->addView(item);
		_modsListItems_[selected_plugin] = item;
	}

	if (plugin_nros_list.empty())
	{

		auto *emptyListLabel = new brls::ListItem(
        "No plugins have been found in " + GlobalObjects::getModBrowser().get_current_directory() + "/.plugins",
			"There you need to put your plugins such as: ./<name-of-the-plugin>.smm");
		emptyListLabel->show([]() {}, false);
		this->addView(emptyListLabel);
	}
}

std::string TabModPlugins::remove_extension(const std::string &filename)
{
	size_t lastdot = filename.find_last_of(".");
	if (lastdot == std::string::npos)
		return filename;
	return filename.substr(0, lastdot);
}

std::string TabModPlugins::get_extension(const std::string &filename)
{
	size_t lastdot = filename.find_last_of(".");
	if (lastdot == std::string::npos)
		return filename;
	return filename.substr(lastdot);
}

brls::Image *TabModPlugins::load_image_cache(std::string filename)
{
	LogDebug("Requesting icon: %s", filename);

	brls::Image *image = nullptr;

	std::string filename_enc = base64_encode(filename);

	std::map<std::string, brls::Image *>::iterator it;

	it = cached_thumbs.find(filename_enc);
	// found
	if (it != cached_thumbs.end())
	{
		LogDebug("Icon Already Cached: %s", filename);
		image = cached_thumbs[filename_enc];
	}
	else
	// not found
	{
		LogDebug("Icon Not Yet Cached: %s", filename);

		FILE *file = fopen(filename.c_str(), "rb");
		if (file)
		{
			fseek(file, sizeof(NroStart), SEEK_SET);
			NroHeader header;
			fread(&header, sizeof(header), 1, file);
			fseek(file, header.size, SEEK_SET);
			NroAssetHeader asset_header;
			fread(&asset_header, sizeof(asset_header), 1, file);

			size_t icon_size = asset_header.icon.size;
			uint8_t *icon = (uint8_t *)malloc(icon_size);
			if (icon != NULL && icon_size != 0)
			{
				memset(icon, 0, icon_size);
				fseek(file, header.size + asset_header.icon.offset, SEEK_SET);
				fread(icon, icon_size, 1, file);

				LogDebug("Caching New Icon: %s", filename);

				image = new brls::Image(icon, icon_size);

				cached_thumbs[filename_enc] = image;
			}
			else
				image = new brls::Image("romfs:/images/unknown.png");

			free(icon);
			icon = NULL;
		}
		else
		{
			LogDebug("Using Unknown Icon For: %s", filename);
			image = new brls::Image("romfs:/images/unknown.png");
		}
	}

	return image;
}

std::string TabModPlugins::base64_encode(const std::string &in)
{
	std::string out;

	int val = 0, valb = -6;
	for (unsigned char c : in)
	{
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0)
		{
			out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6)
		out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4)
		out.push_back('=');
	return out;
}

std::string TabModPlugins::base64_decode(const std::string &in)
{
	std::string out;

	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++)
		T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

	int val = 0, valb = -8;
	for (unsigned char c : in)
	{
		if (T[c] == -1)
			break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0)
		{
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
	return out;
}
void TabModPlugins::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                         brls::FrameContext *ctx)
{

	ScrollView::draw(vg, x, y, width, height, style, ctx);
}
