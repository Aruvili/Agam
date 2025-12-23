import { NavLink } from "react-router-dom"
import { cn } from "../lib/utils"
import { docsConfig } from "../config/docs"

export function Sidebar() {
    return (
        <aside className="fixed top-14 z-30 -ml-2 hidden h-[calc(100vh-3.5rem)] w-full shrink-0 md:sticky md:block">
            <div className="h-full py-6 pr-6 lg:py-8">
                <div className="w-full">
                    {docsConfig.sidebarNav.map((item, index) => (
                        <div key={index} className="pb-4">
                            <h4 className="mb-1 rounded-md px-2 py-1 text-sm font-semibold">
                                {item.title}
                            </h4>
                            {item.items?.length && (
                                <div className="grid grid-flow-row auto-rows-max text-sm">
                                    {item.items.map((subItem, subIndex) => (
                                        <NavLink
                                            key={subIndex}
                                            to={subItem.href}
                                            className={({ isActive }) => cn(
                                                "group flex w-full items-center rounded-md border border-transparent px-2 py-1 hover:underline text-muted-foreground",
                                                isActive ? "font-medium text-foreground" : ""
                                            )}
                                        >
                                            {subItem.title}
                                        </NavLink>
                                    ))}
                                </div>
                            )}
                        </div>
                    ))}
                </div>
            </div>
        </aside>
    )
}
