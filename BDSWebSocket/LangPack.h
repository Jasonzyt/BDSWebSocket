#ifndef LANGPACK_H
#define LANGPACK_H
#include "pch.h"
#include "Mess.h"
#include "nlohmann/json.hpp"

class LangPack {

	std::string langType;
	std::unordered_map<std::string, std::string> lang;
	nlohmann::json default_lang{
		{"en", {
				{"ws.onopen", "WebSocket client connected! Address: %s"},
				{"ws.onclose", "WebSocket client disconnected! Address: %s Reason: %s"},
				{"ws.onerror", "An WebSocket error occurred! Address: %s"},
				{"ws.onreceived", "Received from %s <- %s"},
				{"ws.onsent", "Sent to %s -> %s"},
				{"ws.onstart", "WebSocket server started! Port: %d"},
				{"ws.onstop", "WebSocket server stopped! (Opening time: %s)"},
				{"ws.notallowed", "WebSocket client '%s' tried connecting server, but it isn't in IPWhiteList! Disconnecting..."},
			}
		},
		{"zh-cn", {
				{"ws.onopen", "WebSocket客户端连接! 地址: %s"},
				{"ws.onclose", "WebSocket客户端断开连接! 地址: %s 原因: %s"},
				{"ws.onerror", "出现了一个WebSocket错误! 客户端地址: %s"},
				{"ws.onreceived", "收到来自 %s 的消息 -> %s"},
				{"ws.onsent", "已发送消息至 %s -> %s"},
				{"ws.onstart", "WebSocket服务器已启动! 启动端口: %d"},
				{"ws.onstop", "WebSocket服务器已停止! (持续开启时间: %s)"},
				{"ws.notallowed", "WebSocket客户端'%s'尝试连接服务器, 但它不在IP白名单中! 断开连接中..."},
			}
		}
	};

	inline void writeDefault(const std::string& fn) {
		std::fstream file(fn, std::ios::out | std::ios::ate);
		file << std::setw(4) << default_lang;
		file.close();
	}

	inline void init(const std::string& fn, const std::string& language) {
		nlohmann::json json;
		auto lastSlash = fn.find_last_of('/');
		auto lastBackslash = fn.find_last_of('\\');
		fs::create_directories(fn.substr(0, max((lastSlash != std::string::npos ? lastSlash : 0),
			(lastBackslash != std::string::npos ? lastBackslash : 0))));
		std::fstream file(fn, std::ios::in | std::ios::app);
		std::ostringstream oss;
		bool flag = false;
		oss << file.rdbuf();
		file.close();
		if (oss.str().empty()) {
			writeDefault(fn);
		}

		try {
			json = nlohmann::json::parse(oss.str());
		}
		catch (nlohmann::detail::exception e) {
			if (e.id != 101) {
				Logger::Error() << "Error when parse JSON!!! " << e.what() <<
					" => Content: " << oss.str() << Logger::endl;
			}
			Logger::Info() << "Try rewriting langpack file...\n";
			writeDefault(fn);
			if (!default_lang.count(language)) {
				Logger::Warn() << "Unknown language name, please check config file! Default English\n";
				lang = default_lang.at("en").get<std::unordered_map<std::string, std::string>>();
			}
			else lang = default_lang.at(language).get<std::unordered_map<std::string, std::string>>();
			return;
		}

		if (!json.count(language)) {
			Logger::Error() << "Unknown language name, please check config file! Default English\n";
			lang = default_lang.at("en").get<std::unordered_map<std::string, std::string>>();
			return;
		}
		/*
		bool modified = false;
		for (auto it = default_lang["en"].begin();
			it != default_lang.end(); it++) {
			if (!json.count(it.key())) {
				modified = true;
				if (language == "zh-cn") {
					json.at(language)[it.key()] = default_lang["zh-cn"].at(it.key());
				}
				else {
					json.at(language)[it.key()] = it.value();
				}
			}
		}
		if (modified) {
			std::fstream file(fn, std::ios::out | std::ios::ate);
			file << std::setw(4) << json;
			file.close();
		}*/
		lang = json.at(language).get<std::unordered_map<std::string, std::string>>();
	}

public:

	LangPack(const std::string& file, const std::string& language) {
		langType = language;
		init(file, language);
	}

	inline std::string get(const std::string& key) {
		if (lang.count(key)) {
			return lang.at(key);
		}
		else if (default_lang.count(langType) && default_lang[langType].count(key)) {
			return default_lang[langType][key].get<std::string>();
		}
		Logger::Warn() << "Could not find the translation for " << key << Logger::endl;
		return key;
	}

	template<typename ... Args>
	inline std::string localization(const std::string& key, Args... args) {
		return format(this->get(key), args...);
	}
	
};

#endif // !LANGPACK_H
