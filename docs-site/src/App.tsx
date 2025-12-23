import { BrowserRouter as Router, Routes, Route } from "react-router-dom"
import { Layout } from "./components/Layout"
import { ThemeProvider } from "./components/theme-provider"
import { Home } from "./pages/Home"
import { DocPage } from "./pages/DocPage"

function App() {
  return (
    <ThemeProvider defaultTheme="dark" storageKey="vite-ui-theme">
      <Router>
        <Layout>
          <Routes>
            <Route path="/" element={<Home />} />
            <Route path="/docs" element={<DocPage />} />
            <Route path="/docs/:slug" element={<DocPage />} />
          </Routes>
        </Layout>
      </Router>
    </ThemeProvider>
  )
}

export default App
