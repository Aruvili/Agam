import React from 'react';
import { motion } from 'framer-motion';
import { Sparkles, Code, FileText, Bug, Users, Github, Twitter, Linkedin } from 'lucide-react';

const teamMembers = [
    {
        name: "Balapriyan",
        role: "Founder & Backend Lead",
        subRole: "Team Aruvili",
        icon: Code,
        gradient: "from-blue-500 to-cyan-500",
        delay: 0.1
    },
    {
        name: "Bagavathi Singh B",
        role: "Frontend Lead",
        subRole: "Documentation",
        icon: FileText,
        gradient: "from-emerald-500 to-green-500",
        delay: 0.2
    },
    {
        name: "Sriram G",
        role: "Junior Backend Developer",
        subRole: "QA & Testing",
        icon: Bug,
        gradient: "from-orange-500 to-yellow-500",
        delay: 0.3
    },
    {
        name: "Claude Sonnet 4.5",
        role: "AI Assistant",
        subRole: "Development Partner",
        icon: Sparkles,
        gradient: "from-purple-500 to-pink-500",
        delay: 0.4
    }
];

const TeamMemberCard = ({ member }) => {
    const Icon = member.icon;

    return (
        <motion.div
            initial={{ opacity: 0, y: 20 }}
            whileInView={{ opacity: 1, y: 0 }}
            viewport={{ once: true }}
            transition={{ duration: 0.5, delay: member.delay }}
            className="group relative h-full"
        >
            <div className="absolute -inset-0.5 bg-gradient-to-br opacity-30 transition-all duration-300 group-hover:opacity-60"
                style={{ backgroundImage: `linear-gradient(to bottom right, var(--tw-gradient-stops))` }}
            >
                <div className={`w-full h-full bg-gradient-to-br ${member.gradient} opacity-20`}></div>
            </div>

            <div className="relative h-full bg-zinc-950 border border-zinc-900 rounded-2xl p-6 overflow-hidden hover:border-zinc-700 transition-colors duration-300 flex flex-col items-center text-center">
                {/* Background Gradient Blob */}
                <div className={`absolute top-0 inset-x-0 h-px bg-gradient-to-r ${member.gradient} opacity-0 group-hover:opacity-100 transition-opacity duration-500`} />
                <div className={`absolute -top-24 -right-24 w-48 h-48 bg-gradient-to-br ${member.gradient} opacity-[0.03] group-hover:opacity-[0.08] rounded-full transition-opacity duration-500`} />

                {/* Icon */}
                <div className="relative mb-6">
                    <div className={`absolute inset-0 bg-gradient-to-br ${member.gradient} opacity-20 group-hover:opacity-40 transition-opacity duration-300`} />
                    <div className="relative w-16 h-16 rounded-2xl bg-zinc-900/80 border border-zinc-800 flex items-center justify-center group-hover:scale-105 transition-transform duration-300">
                        <Icon className="text-zinc-100" size={32} />
                    </div>
                </div>

                {/* Content */}
                <h3 className="text-lg font-bold text-zinc-100 mb-1 group-hover:text-transparent group-hover:bg-clip-text group-hover:bg-gradient-to-r group-hover:from-white group-hover:to-zinc-400 transition-all">
                    {member.name}
                </h3>
                <div className="text-sm font-medium text-zinc-400 mb-1">{member.role}</div>
                <div className="text-xs text-zinc-600 font-mono uppercase tracking-wider">{member.subRole}</div>

                {/* Decorative Dots */}
                <div className="mt-6 flex gap-2">
                    <div className="w-1.5 h-1.5 rounded-full bg-zinc-800 group-hover:bg-zinc-600 transition-colors" />
                    <div className="w-1.5 h-1.5 rounded-full bg-zinc-800 group-hover:bg-zinc-600 transition-colors delay-75" />
                    <div className="w-1.5 h-1.5 rounded-full bg-zinc-800 group-hover:bg-zinc-600 transition-colors delay-150" />
                </div>
            </div>
        </motion.div>
    );
};

const Team = () => {
    return (
        <section className="relative mt-32 pb-20 border-t border-zinc-900/50">

            <div className="absolute inset-0 bg-gradient-to-b from-zinc-950 via-zinc-900/20 to-zinc-950 pointer-events-none" />

            <div className="relative max-w-7xl mx-auto px-6">
                <motion.div
                    initial={{ opacity: 0, y: 20 }}
                    whileInView={{ opacity: 1, y: 0 }}
                    viewport={{ once: true }}
                    className="text-center mb-16"
                >
                    <span className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-zinc-900 border border-zinc-800 text-xs font-medium text-zinc-400 mb-6">
                        <Users size={14} className="text-blue-400" />
                        <span>The Minds Behind Agam</span>
                    </span>

                    <h2 className="text-3xl md:text-5xl font-bold text-white mb-6 tracking-tight">
                        Built by <span className="text-transparent bg-clip-text bg-gradient-to-r from-blue-400 via-purple-400 to-pink-400">Team Aruvili</span>
                    </h2>

                    <p className="text-lg text-zinc-400 max-w-2xl mx-auto leading-relaxed">
                        We are a passionate team of developers dedicated to breaking language barriers in programming.
                        Agam is our contribution to the Tamil open-source community.
                    </p>
                </motion.div>

                <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6 lg:gap-8">
                    {teamMembers.map((member) => (
                        <TeamMemberCard key={member.name} member={member} />
                    ))}
                </div>

                <motion.div
                    initial={{ opacity: 0 }}
                    whileInView={{ opacity: 1 }}
                    viewport={{ once: true }}
                    className="mt-20 text-center"
                >
                    <div className="inline-flex items-center gap-6 text-zinc-600 text-sm">
                        <span className="flex items-center gap-2 hover:text-zinc-400 transition-colors cursor-pointer">
                            <Github size={18} />
                            <span>Contribute</span>
                        </span>
                        <div className="w-1 h-1 rounded-full bg-zinc-800" />
                        <span className="flex items-center gap-2 hover:text-zinc-400 transition-colors cursor-pointer">
                            <Twitter size={18} />
                            <span>Updates</span>
                        </span>
                    </div>
                </motion.div>
            </div>
        </section>
    );
};

export default Team;
