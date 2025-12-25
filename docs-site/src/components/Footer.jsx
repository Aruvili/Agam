import React from 'react';

const Footer = () => {
    return (
        <footer className="mt-20 pt-8 border-t border-zinc-800 text-center text-zinc-600">
            <p>&copy; {new Date().getFullYear()} All copy rights reserved by Aruvili</p>
        </footer>
    );
};

export default Footer;
