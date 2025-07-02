#include <drx3D/Network2/Http/HttpResponse.h>

#include <limits>

#include <drx3D/Common/String.h>

namespace drx3d {
HttpResponse::HttpResponse() :
	status(Status::ConnectionFailed),
	majorVersion(0),
	minorVersion(0) {
}

STxt HttpResponse::GetField(const STxt &field) const {
	auto it = fields.find(String::Lowercase(field));
	if (it != fields.end())
		return it->second;
	return {};
}

void HttpResponse::Parse(const STxt &data) {
	std::istringstream in(data);

	// Extract the HTTP version from the first line.
	STxt version;

	if (in >> version) {
		if (version.size() >= 8 && version[6] == '.' && String::Lowercase(version.substr(0, 5)) == "http/" && isdigit(version[5]) && isdigit(version[7])) {
			majorVersion = version[5] - '0';
			minorVersion = version[7] - '0';
		} else {
			// Invalid HTTP version.
			status = Status::InvalidResponse;
			return;
		}
	}

	// Extract the status code from the first line.
	i32 status;

	if (in >> status) {
		this->status = static_cast<Status>(status);
	} else {
		// Invalid status code.
		this->status = Status::InvalidResponse;
		return;
	}

	// Ignore the end of the first line.
	in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	// Parse the other lines, which contain fields, one by one.
	ParseFields(in);

	body.clear();

	// Determine whether the transfer is chunked.
	if (String::Lowercase(GetField("transfer-encoding")) != "chunked") {
		// Not chunked - just read everything at once.
		std::copy(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>(), std::back_inserter(body));
	} else {
		// Chunked - have to read chunk by chunk.
		std::size_t length;

		// Read all chunks, identified by a chunk-size not being 0.
		while (in >> std::hex >> length) {
			// Drop the rest of the line (chunk-extension).
			in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			// Copy the actual content data.
			std::istreambuf_iterator<char> it = in;
			std::istreambuf_iterator<char> itEnd;

			for (std::size_t i = 0; i < length && it != itEnd; i++)
				body.push_back(*it++);
		}

		// Drop the rest of the line (chunk-extension).
		in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		// Read all trailers (if present).
		ParseFields(in);
	}
}

void HttpResponse::ParseFields(std::istream &in) {
	STxt line;

	while (std::getline(in, line) && line.size() > 2) {
		auto pos = line.find(": ");

		if (pos != STxt::npos) {
			// Extract the field name and its value.
			auto field = line.substr(0, pos);
			auto value = line.substr(pos + 2);

			// Remove any trailing '\r'.
			if (!value.empty() && (*value.rbegin() == '\r'))
				value.erase(value.size() - 1);

			// Add the field.
			fields[String::Lowercase(field)] = value;
		}
	}
}
}
