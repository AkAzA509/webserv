/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 12:33:21 by macorso           #+#    #+#             */
/*   Updated: 2025/08/30 16:43:51 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.h"
#include "Logger.h"

Response::Response() : m_server(NULL), m_request(NULL) {}

Response::Response(Request& req, Server& server) : m_server(&server), m_request(&req)
{
	buildResponse();
}

void Response::setDefaultResponse()
{
	m_firstline = HEADER_OK;
}

std::pair<std::string, std::string> Response::getError(int page, const std::string& page_path) const
{
	switch (page)
	{
	case 404:
		return std::make_pair(ERROR_404, loadFile(buildPath(page_path)));
	case 403:
		return std::make_pair(ERROR_403, loadFile(buildPath(page_path)));
	case 400:
		return std::make_pair(ERROR_400, loadFile(buildPath(page_path)));
	case 405:
		return std::make_pair(ERROR_405, loadFile(buildPath(page_path)));
	case 411:
		return std::make_pair(ERROR_411, loadFile(buildPath(page_path)));
	case 500:
		return std::make_pair(ERROR_500, loadFile(buildPath(page_path)));
	default:
		return std::make_pair(ERROR_500, loadFile(buildPath(page_path)));
	}
}

// Utile pour éviter les doubles slashs
std::string Response::joinPaths(const std::string& base, const std::string& relative) const
{
	if (base.empty()) return relative;
	if (relative.empty()) return base;
	if (base[base.size() - 1] == '/' && relative[0] == '/')
		return base + relative.substr(1);
	if (base[base.size() - 1] != '/' && relative[0] != '/')
		return base + "/" + relative;
	return base + relative;
}


std::string Response::buildPath(const std::string& page_path) const
{
    std::string root = normalizePath(m_server->getRoot());
    std::string page_path_normalized;

    // Si la requête est "/" ou vide, on sert le premier index défini
    if (page_path == "/" || page_path.empty()) {
        std::vector<std::string> indexes = getLocationOrServerIndexes();
        page_path_normalized = normalizePath(indexes.empty() ? "index.html" : indexes[0]);
    } else {
        page_path_normalized = normalizePath(page_path);
    }

    std::string full_path = joinPaths(root, page_path_normalized);
    Logger::log(YELLOW, "root path norm : %s, path norm : %s, full path : %s\n", root.c_str(), page_path_normalized.c_str(), full_path.c_str());
    return full_path;
}

std::vector<std::string> Response::getLocationOrServerIndexes() const
{
    const Location& loc = m_request->getLocation();
    std::vector<std::string> locIndexes = loc.getIndexFiles();
    if (!locIndexes.empty())
        return locIndexes;

    std::vector<std::string> serverIndexes = m_server->getIndexFiles();
    if (!serverIndexes.empty())
        return serverIndexes;

    // Par défaut, si aucun index défini, on essaye "index.html"
    return std::vector<std::string>(1, "index.html");
}

std::string Response::getFullResponse() const
{
    std::ostringstream oss;
    // Status line (ex: "HTTP/1.1 200 OK")
    oss << m_firstline;

    // Content-Type
    std::string contentType;
    if (!m_servedFilePath.empty())
        contentType = getMimeType(m_servedFilePath);
    else
        contentType = "text/html"; // Par défaut pour les erreurs ou autoindex

    oss << "Content-Type: " << contentType << "\r\n";

    // Content-Length
    oss << "Content-Length: " << m_body.size() << "\r\n";

    // Server header (optionnel mais bien)
    oss << "Server: Webserv/1.0\r\n";
    oss << "Connection: close\r\n";

    // Headers personnalisés éventuels
    for (std::map<std::string, std::string>::const_iterator it = m_header.begin(); it != m_header.end(); ++it)
        oss << it->first << ": " << it->second << "\r\n";

    // Ligne vide
    oss << "\r\n";
    // Body
    oss << m_body;

    return oss.str();
}

// Mets à jour m_servedFilePath dans handleGet :
void Response::handleGet()
{
    Logger::log(YELLOW, "path : %s\n", m_request->getPath().c_str());
    std::string filePath = buildPath(m_request->getPath());
    Logger::log(YELLOW, "path builder : %s\n", filePath.c_str());

    struct stat st;
    if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        if (m_request->getIsAutoIndex()) {
            m_firstline = HEADER_OK;
            m_body = m_request->getAutoIndex();
            m_servedFilePath.clear(); // autoindex = pas de fichier
            return;
        }
        std::vector<std::string> indexes = getLocationOrServerIndexes();
        for (size_t i = 0; i < indexes.size(); ++i) {
            std::string indexFile = joinPaths(filePath, indexes[i]);
            std::string body = loadFile(indexFile);
            if (!body.empty()) {
                m_firstline = HEADER_OK;
                m_body = body;
                m_servedFilePath = indexFile;
                return;
            }
        }
        setErrorResponse(404);
        m_servedFilePath.clear();
        return;
    }

    std::string body = loadFile(filePath);

    if (!body.empty()) {
        m_firstline = HEADER_OK;
        m_body = body;
        m_servedFilePath = filePath;
    } else {
        setErrorResponse(404);
        m_servedFilePath.clear();
    }
}

// Mets à jour m_servedFilePath dans setErrorResponse :
void Response::setErrorResponse(int errorCode)
{
    const std::map<int, std::string>& pages = m_server->getErrorPages();
    std::map<int, std::string>::const_iterator it = pages.find(errorCode);

    if (it != pages.end()) {
        std::pair<std::string, std::string> page = getError(errorCode, it->second);
        m_firstline = page.first;
        m_body = page.second;
        m_servedFilePath = it->second; // pour le Content-Type
    } else {
        switch (errorCode) {
            case 400: m_firstline = ERROR_400; m_body = P_ERROR_400; break;
            case 403: m_firstline = ERROR_403; m_body = P_ERROR_403; break;
            case 404: m_firstline = ERROR_404; m_body = P_ERROR_404; break;
            case 405: m_firstline = ERROR_405; m_body = P_ERROR_405; break;
            case 411: m_firstline = ERROR_411; m_body = P_ERROR_411; break;
            case 500: m_firstline = ERROR_500; m_body = P_ERROR_500; break;
            default: m_firstline = ERROR_500; m_body = P_ERROR_500; break;
        }
        m_servedFilePath.clear();
    }
}

// Pour POST/PUT/DELETE, fais pareil : après avoir généré le fichier, mets m_servedFilePath au chemin du fichier créé.
// Exemple pour POST :
void Response::handlePost()
{
    std::string root = m_request->getLocation().getRoot();
    std::string uploadPath = m_request->getLocation().getUploadPath();
    const std::vector<BinaryInfo>& binaries = m_request->getBinaryInfos();

    if (!binaries.empty()) {
        bool all_success = true;
        for (size_t i = 0; i < binaries.size(); ++i) {
            const BinaryInfo& bin = binaries[i];
            std::string filePath = normalizePath(root + "/" + uploadPath + "/" + bin.filename);

            std::ofstream outfile(filePath.c_str(), std::ios::binary);
            if (!outfile) {
                all_success = false;
                setErrorResponse(500);
                m_servedFilePath.clear();
                return;
            }
            outfile.write(bin.data.c_str(), bin.data.size());
            outfile.close();
            m_servedFilePath = filePath; // <- MAJ ici
        }
        if (all_success) {
            m_firstline = HEADER_201;
            m_body = "File(s) uploaded successfully.";
        }
    }
    // Cas 2 : POST simple (texte, json, etc.)
    else if (!m_request->getRawBody().empty()) {
        std::string filePath = normalizePath(root + "/" + uploadPath + "/post_body.txt");
        std::ofstream outfile(filePath.c_str(), std::ios::binary);
        if (!outfile) {
            setErrorResponse(500);
            m_servedFilePath.clear();
            return;
        }
        const std::string& body = m_request->getRawBody();
        outfile.write(body.c_str(), body.size());
        outfile.close();
        m_servedFilePath = filePath; // <- MAJ ici
        m_firstline = HEADER_201;
        m_body = "Body posted successfully.";
    }
    // Cas 3 : rien à poster
    else {
        setErrorResponse(400);
        m_servedFilePath.clear();
    }
}

// Fait pareil dans handlePut, handleDelete si tu écris/supprimes des fichiers

void Response::handleDelete()
{
    std::string filePath = buildPath(m_request->getPath());
    std::vector<std::string> cgiPass = m_request->getLocation().getCgiPath();
    if (cgiPass.empty()) {
        setErrorResponse(500);
        m_body = "CGI script for DELETE not configured.";
        return;
    }
    std::string cgiScript = cgiPass[0];
    std::string cgiOutput;
    std::vector<std::string> args;
    args.push_back(filePath);
    std::vector<std::string> extraEnv;
    extraEnv.push_back("REQUEST_METHOD=DELETE");
    extraEnv.push_back("PATH_INFO=" + filePath);
    bool success = m_request->doCGI(cgiScript, args, extraEnv, cgiOutput);
    if (success && cgiOutput.find("success") != std::string::npos) {
        m_firstline = HEADER_OK;
        m_body = cgiOutput;
    } else {
        setErrorResponse(500);
        m_body = cgiOutput.empty() ? "Error deleting file." : cgiOutput;
    }
}

void Response::handlePut()
{
	// Chemin complet demandé par le client
	std::string root = m_request->getLocation().getRoot();
	std::string fullPath = normalizePath(root + m_request->getPath());

	// 1. Donnée à écrire (multipart ou body brut)
	const std::vector<BinaryInfo>& binaries = m_request->getBinaryInfos();

	// Cas 1 : PUT multipart (rare, mais possible)
	if (!binaries.empty()) {
		// En général on s'attend à un seul fichier
		const BinaryInfo& bin = binaries[0];
		std::ofstream outfile(fullPath.c_str(), std::ios::binary);
		if (!outfile) {
			setErrorResponse(500);
			return;
		}
		outfile.write(bin.data.c_str(), bin.data.size());
		outfile.close();
		m_firstline = HEADER_201;
		m_body = "File replaced/uploaded successfully by PUT.";
	}
	// Cas 2 : PUT body brut (le plus classique)
	else if (!m_request->getRawBody().empty()) {
		const std::string& body = m_request->getRawBody();
		std::ofstream outfile(fullPath.c_str(), std::ios::binary);
		if (!outfile) {
			setErrorResponse(500);
			return;
		}
		outfile.write(body.c_str(), body.size());
		outfile.close();
		m_firstline = HEADER_201;
		m_body = "Resource replaced/uploaded successfully by PUT.";
	}
	else
		setErrorResponse(400);
}

void Response::buildResponse()
{
	std::string method = m_request->getMethod();
	setDefaultResponse();

	if (m_request->IsErrorPage()) {
		const std::map<int, std::string>& pages = m_server->getErrorPages();
		std::map<int, std::string>::const_iterator it = pages.find(m_request->getErrorPage());

		if (it != pages.end()) {
			std::pair<std::string, std::string> page = getError(it->first, it->second);
			m_firstline = page.first;
			m_body = page.second;
		}
		else {
			m_firstline = ERROR_500;
			m_body = P_ERROR_500;
		}
	}
	else {
        if (!m_request->getLocation().isAllowedMethod(m_request->getMethod())) {
            Logger::log(RED, "Method %s not allowed for path %s", m_request->getMethod().c_str(), m_request->getPath().c_str());
            setErrorResponse(405);
        }
		if (m_request->getMethod().find("cgi-bin") != std::string::npos)
			// m_request->doCGI(a changer ca)
			;
		else {
			if (method == "GET")
				handleGet();
			else if (method == "POST")
				handlePost();
			else if (method == "DELETE")
				handleDelete();
			else if (method == "PUT")
				handlePut();
			else
				setErrorResponse(500);
		}
	}
	// Logger::log(RED, "Body: %s\n", m_body.c_str());
}