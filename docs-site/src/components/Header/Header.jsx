import React from "react";
import { FaGithub } from "react-icons/fa";

export default function Header() {
  return (
    <header className="fixed top-0 left-0 right-0 z-50 backdrop-blur-md bg-white/10 border-b border-white/20 shadow-lg">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-16">
          {/* Left Section */}
          <div className="flex items-center gap-4">
            <img
              src="/logo192.png"
              alt="Logo"
              className="h-10 w-10 rounded-lg"
            />
            <input
              className="hidden sm:block px-4 py-2 rounded-lg bg-white/20 backdrop-blur-sm border border-white/30 text-white placeholder-white/60 focus:outline-none focus:ring-2 focus:ring-purple-400/50 transition-all"
              type="text"
              placeholder="Search..."
            />
          </div>

          {/* Center Section */}
          <div className="absolute left-1/2 transform -translate-x-1/2">
            <span className="relative inline-block px-6 py-2 text-xl font-semibold text-white">
       
              <span className="relative bg-gradient-to-r from-emerald-300 via-purple-400 to-indigo-400 bg-clip-text text-transparent">
                அகம் (Agam)
              </span>
            </span>
          </div>

          {/* Right Section */}
          <nav className="flex items-center gap-4">
            <a
              href="https://github.com"
              target="_blank"
              rel="noopener noreferrer"
              className="p-2 rounded-lg text-white hover:bg-white/20 transition-all duration-200"
            >
              <FaGithub size={22} />
            </a>

            <a
              href="/login"
              className="px-4 py-2 rounded-lg bg-white/20 backdrop-blur-sm border border-white/30 text-white font-medium hover:bg-white/30 transition-all duration-200"
            >
              Log In
            </a>
          </nav>
        </div>
      </div>
    </header>
  );
}
