#pragma once

#include <map>
#include <string>

struct MIME_Types
{
	std::map<std::string, std::string> MIME_map =
	{
		{"txt" , "text/plain"     },
		{"html", "text/html"      },
		{"htm" , "text/html"      },
		{"css" , "text/css"       },
		{"js"  , "text/javascript"},
		{"gif" , "image/gif"      },
		{"jpeg", "image/jpeg"     },
		{"jpg" , "image/jpeg"     },
		{"png" , "image/png"      },
		{"mp3" , "audio/mpeg"     },
		{"oga" , "audio/ogg"      },
		{"wav" , "audio/wav"      },
		{"mp4" , "video/mpeg"     },
		{"mpeg", "video/mpeg"     },
		{"ogv" , "video/ogg"      },
	};
};

