/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 10:13:39 by ggirault          #+#    #+#             */
/*   Updated: 2025/08/02 11:31:03 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.h"
#include "Parser.h"
#include "Logger.h"

Request::Request(Location loc, std::vector<std::string>& firstRequestLine, std::vector<std::string>& request, std::string& full_request, char **env)
: m_loc(loc), m_methode(firstRequestLine[0]), m_url(firstRequestLine[1]), m_version(firstRequestLine[2]), m_foundBody(false), m_errorPage(false), m_responseStatus(HEADER_OK), m_env(env) {
	if (!m_loc.isAllowedMethode(m_methode)) {
		setError(ERROR_405, "methode " + m_methode + " not autorized for this location");
		return;
	}
	parseHeaders(request);
	if (m_methode == "GET")
		methodeGet(request, full_request);
	else if (m_methode == "POST")
		methodePost(request, full_request);
	else if (m_methode == "DELETE")
		methodeDelete(request, full_request);
	else if (m_methode == "PUT")
		methodePut(request, full_request);
	else
		setError(ERROR_405, "Unknown HTTP method: " + m_methode);
}

Request::Request(const Request &copy) { *this = copy; }

Request& Request::operator=(const Request &other) {
	if (this != &other) {
		m_loc = other.m_loc;
		m_methode = other.m_methode;
		m_url = other.m_url;
		m_version = other.m_version;
		m_content_type = other.m_content_type;
		m_content_length = other.m_content_length;
		m_body_reponse = other.m_body_reponse;
		m_body_request = other.m_body_request;
		m_foundBody = other.m_foundBody;
		m_errorPage = other.m_errorPage;
		m_responseStatus = other.m_responseStatus;
		m_headers = other.m_headers;
		m_env = other.m_env;
	}
	return *this;
}

std::vector<std::string> Request::convertEnv() {
	std::vector<std::string> env;
	for (size_t i = 0; m_env[i]; i++)
		env.push_back(m_env[i]);
	return env;
}

// void Request::doCGI(size_t end_header, std::string& request) {
// 	std::vector<std::string> env = convertEnv();
// 	// char *av[] = { (char *)m_url, NULL };
// 	int fd[2];

// 	env.push_back("REQUEST_METHOD=" + m_methode);
// 	env.push_back("CONTENT_LENGTH=" + request.substr(end_header).size());
// 	env.push_back("SERVER_PROTOCOL=HTTP/1.1");

// 	std::vector<char *> new_env;
// 	for (size_t i = 0; i < env.size(); ++i)
// 		new_env.push_back(const_cast<char *>(env[i].c_str()));
// 	new_env.push_back(NULL);

// 	if (!pipe(fd)) {
// 		Logger::log(RED, "error pipe : pipe failed");
// 		return ;
// 	}
// 	pid_t pid = fork();
// 	if (pid == 0) {
// 		// dup2();
// 		execve("./cgi-bin/change_color.sh", av, new_env.data());
// 	}
// }

void Request::doCGI(size_t end_header, std::string& request) {
	std::vector<std::string> env = convertEnv();

	env.push_back("REQUEST_METHOD=" + m_methode);
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");

	std::string body = request.substr(end_header);

	if (m_methode == "GET") {
		size_t query_pos = m_url.find('?');
		std::string query = (query_pos != std::string::npos) ? m_url.substr(query_pos + 1) : "";
		env.push_back("QUERY_STRING=" + query);
	} else if (m_methode == "POST") {
		std::stringstream ss;
		ss << body.size();
		std::string len = ss.str();
		env.push_back("CONTENT_LENGTH=" + len);
	}

	std::vector<char *> envp;
	for (size_t i = 0; i < env.size(); ++i)
		envp.push_back(const_cast<char *>(env[i].c_str()));
	envp.push_back(NULL);
	m_url.erase(0, 1);
	char *av[] = { const_cast<char *>(m_url.c_str()), NULL };
	std::cout << "url = " << av[0] << std::endl;
	int pipe_in[2];
	int pipe_out[2];
	if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1) {
		Logger::log(RED, "pipe failed");
		return;
	}

	pid_t pid = fork();
	if (pid < 0) {
		Logger::log(RED, "fork failed");
		return;
	}
	if (pid == 0) {
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_out[0]);

		if (m_methode == "POST")
			dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_in[1]);

		execve(av[0], av, envp.data());
		exit(EXIT_FAILURE);
	}
	else {
		close(pipe_out[1]);

		if (m_methode == "POST")
			write(pipe_in[1], body.c_str(), body.size());
		close(pipe_in[1]);
		close(pipe_in[0]);

		char buffer[1024];
		ssize_t bytes;
		std::string cgi_output;
		while ((bytes = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
			cgi_output.append(buffer, bytes);
		}
		close(pipe_out[0]);

		int status;
		if (waitpid(pid, &status, 0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			setError(ERROR_500, "cgi execution failed !");
			return;
		}

		Logger::log(WHITE, "CGI Output:\n%s\n", cgi_output.c_str());
		m_body_reponse = cgi_output;
	}
}

// parsing 1 :

// - recuperer la requette et checker la location si cgi ou pas , si oui la stocker et executer sinon 
// parser et recuperer les parametres

// 2 :

// en fonction de la methode la faire, (uploader un fichier, download un fichier, delete un fichier, et put un fichier a un chemin precis)

// 3 :

// stcoker les reponse dans la var body de request et faire la reponse en fonction de la methode

bool Request::methodePost(std::vector<std::string>&, std::string& full_request) {
	size_t header_end = full_request.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return setError(ERROR_400, "header not complete");

	// if (m_url.find("cgi-bin") != std::string::npos) {
	// 	doCGI(header_end + 4, full_request);
	// 	return true;
	// }
	std::cout << "coucou\n";
	if (m_loc.isAutoIndexOn()) {
		std::cout << "resr1\n";
		autoIndex();
		return true;
	}

	std::string headers = full_request.substr(0, header_end);
	std::string body = full_request.substr(header_end + 4);

	m_content_length = getHeader("Content-Length");
	m_content_type = getHeader("Content-Type");
	if (m_content_length.empty() || std::atoi(m_content_length.c_str()) == 0)
		return setError(ERROR_411, "missing or zero Content-Length");

	std::string filename, file_data;
	if (!parseBody(m_content_type, full_request, filename, file_data))
		return false;
	m_responseStatus = HEADER_201;
	writeFile(filename, file_data);
	return true;
}

bool Request::methodePut(std::vector<std::string>&, std::string& full_request) {
	size_t header_end = full_request.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return setError(ERROR_400, "header not complete");

	std::string headers = full_request.substr(0, header_end);
	std::string body = full_request.substr(header_end + 4);

	m_content_length = getHeader("Content-Length");
	m_content_type = getHeader("Content-Type");
	if (m_content_length.empty() || std::atoi(m_content_length.c_str()) == 0)
		return setError(ERROR_411, "missing or zero Content-Length");

	std::string filename, file_data;
	if (!parseBody(m_content_type, full_request, filename, file_data))
		return false;
	m_responseStatus = HEADER_201;
	writeFile(filename, file_data);
	return true;
}

bool Request::methodeGet(std::vector<std::string>& tab, std::string&) {
	for (size_t i = 1; i < tab.size(); ++i) {
		std::vector<std::string> words = split(tab[i], " ");
		if (words.size() > 1) {
			if (words[0] == "Content-Length:")
				m_content_length = words[1];
			else if (words[0] == "Content-Type:")
				m_content_type = words[1];
		}
	}
	return true;
}

bool Request::methodeDelete(std::vector<std::string>&, std::string&) {
	std::cout << "coucou Delete\n";
	return true;
}

bool Request::writeFile(std::string& filename, std::string& file_data) {
	std::string path = m_loc.getPath() + "/" + filename;
	if (path[0] == '/')
		path.erase(0, 1);

	if (access(path.c_str(), F_OK) == 0 && access(path.c_str(), W_OK) != 0)
		return setError(ERROR_500, "permission denied to open " + filename);
	if (access(path.c_str(), F_OK) == 0)
		m_responseStatus = HEADER_OK;

	std::ofstream out(path.c_str(), std::ios::binary | std::ios::trunc);
	if (!out.is_open())
		return setError(ERROR_404, "the location path not existing upload failed !");

	out.write(file_data.data(), file_data.size());
	out.close();
	return true;
}

bool Request::parseBody(const std::string& content_type, const std::string& body, std::string& filename, std::string& file_data) {
    std::string boundary_key = "boundary=";
    size_t b_pos = content_type.find(boundary_key);
    if (b_pos == std::string::npos)
        return setError(ERROR_400, "boundary not found in header");

    std::string boundary = "--" + content_type.substr(b_pos + boundary_key.size());
    boundary.erase(boundary.find_last_not_of("\r\n") + 1);

    size_t part_start = body.find(boundary);
    if (part_start == std::string::npos)
        return setError(ERROR_400, "boundary not found in body");
    part_start += boundary.length() + 2;

    size_t part_header_end = body.find("\r\n\r\n", part_start);
    if (part_header_end == std::string::npos)
        return setError(ERROR_400, "missing Content-disposition");

    std::string part_header = body.substr(part_start, part_header_end - part_start);
    size_t fname_pos = part_header.find("filename=\"");
    if (fname_pos != std::string::npos) {
        fname_pos += 10;
        size_t fname_end = part_header.find("\"", fname_pos);
        if (fname_end != std::string::npos)
            filename = part_header.substr(fname_pos, fname_end - fname_pos);
    }

    std::string end_boundary = boundary + "--";
    size_t data_start = part_header_end + 4;
    size_t data_end = body.find(end_boundary, data_start);
    if (data_end == std::string::npos)
        return setError(ERROR_400, "end boundary not found");
    if (data_end >= 2 && body.substr(data_end - 2, 2) == "\r\n")
        data_end -= 2;

    file_data = body.substr(data_start, data_end - data_start);
    return true;
}

bool Request::setError(std::string code, const std::string& msg) {
	Logger::log(WHITE, "error : %s", msg.c_str());
	m_errorPage = true;
	m_responseStatus = code;
	return false;
}

std::string Request::getHeader(const std::string& name) const {
	std::map<std::string, std::string>::const_iterator it = m_headers.find(name);
	if (it != m_headers.end())
		return it->second;
	return "";
}

void Request::parseHeaders(const std::vector<std::string>& request) {
	for (size_t i = 1; i < request.size(); ++i) {
		const std::string& line = request[i];
		if (line.empty()) break;
		size_t colon_pos = line.find(':');
		if (colon_pos != std::string::npos) {
			std::string name = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1);
			size_t start = value.find_first_not_of(" \t");
			size_t end = value.find_last_not_of(" \t");
			if (start != std::string::npos && end != std::string::npos)
				value = value.substr(start, end - start + 1);
			m_headers[name] = value;
		}
	}
}

std::ostream& operator<<(std::ostream& o, Request& req) {
	o << "=================================\n" << "Method: " << req.getMethod() << "\nContent-Length: " << req.getContentLength() << 
	"\nContent-Type: " << req.getContentType() <<
	"\nUrl: " << req.getUrl() << 
	"\nVersion: " << req.getVersion() <<
	"\nBody: " << req.getBody() <<
	"\n------------------\n" << req.getLocation() << "\n----------------------------------------------\n" << "=================================";
	return o;
}



void Request::autoIndex() {
    std::string dir_path = m_url;
    
    if (dir_path == "/") {
        dir_path = ".";
    } else {
        dir_path.erase(0, 1);
    }
    
    DIR* directory = opendir(dir_path.c_str());
    if (!directory) {
        Logger::log(RED, "Failed to open directory: %s", dir_path.c_str());
        return;
    }
    
    struct dirent* entry;
    std::string page;
    
    // HTML header
    page += "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n";
    page += "<title>Index of " + m_url + "</title>\n";
    page += "<style>\nbody { font-family: sans-serif; max-width: 800px; margin: 40px auto; }\n";
    page += "table { border-collapse: collapse; width: 100%; }\n";
    page += "th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n";
    page += "a { text-decoration: none; color: #0066cc; }\n";
    page += "a:hover { text-decoration: underline; }\n</style>\n</head>\n";
    
    page += "<body>\n<h1>Index of " + m_url + "</h1>\n";
    page += "<table>\n<tr><th>Name</th><th>Size</th></tr>\n";
    
    // Parent directory link
    if (m_url != "/") {
        page += "<tr><td><a href=\"../\">../</a></td><td>-</td></tr>\n";
    }
    
    // Directory contents
    while ((entry = readdir(directory)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;
        
    std::string filename = entry->d_name;
    Logger::log(WHITE, "Processing file: %s", filename.c_str());
    
    std::string fullpath = (dir_path == ".") ? filename : dir_path + "/" + filename;
    
    struct stat fileStat;
    if (stat(fullpath.c_str(), &fileStat) == 0) {
        Logger::log(WHITE, "Building HTML for: %s", filename.c_str());
        
        page += "<tr><td><a href=\"" + filename;
        if (S_ISDIR(fileStat.st_mode))
            page += "/";
        page += "\">" + filename;
        if (S_ISDIR(fileStat.st_mode))
            page += "/";
        page += "</a></td>";
        
        if (S_ISDIR(fileStat.st_mode)) {
            page += "<td>-</td>";
        } else {
            std::stringstream ss;
            ss << fileStat.st_size;
            std::string size_fileStat(ss.str());
            page += "<td>" + size_fileStat + " bytes</td>";
            Logger::log(WHITE, "File size: %s bytes", size_fileStat.c_str());
        }
        page += "</tr>\n";
        
        Logger::log(WHITE, "HTML built successfully for: %s", filename.c_str());
    } else {
        Logger::log(RED, "stat() failed for: %s", filename.c_str());
    }
}

Logger::log(WHITE, "Loop finished, closing tags...");
page += "</table>\n</body>\n</html>";
    closedir(directory);
	std::cout << "page :" << page << std::endl;
}