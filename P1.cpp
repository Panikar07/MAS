#include <bits/stdc++.h>
using namespace std;

struct Point {
    double x = 0.0, y = 0.0;
    Point operator+(const Point& o) const { return {x + o.x, y + o.y}; }
    Point operator-(const Point& o) const { return {x - o.x, y - o.y}; }
    Point operator*(double s) const { return {x * s, y * s}; }
};

struct Edge { int u, v; };

static double norm(const Point& p) { return hypot(p.x, p.y); }

struct Image {
    int W, H;
    vector<unsigned char> pix;
    Image(int w, int h) : W(w), H(h), pix(3 * w * h, 255) {}

    void setPixel(int x, int y, int r, int g, int b) {
        if (x < 0 || x >= W || y < 0 || y >= H) return;
        int id = 3 * (y * W + x);
        pix[id] = (unsigned char)clamp(r, 0, 255);
        pix[id + 1] = (unsigned char)clamp(g, 0, 255);
        pix[id + 2] = (unsigned char)clamp(b, 0, 255);
    }

    void line(int x0, int y0, int x1, int y1, int r, int g, int b) {
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        while (true) {
            setPixel(x0, y0, r, g, b);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void circle(int cx, int cy, int rad, int r, int g, int b, bool fill = false) {
        for (int yy = -rad; yy <= rad; ++yy) {
            for (int xx = -rad; xx <= rad; ++xx) {
                int d2 = xx * xx + yy * yy;
                if ((fill && d2 <= rad * rad) || (!fill && d2 <= rad * rad && d2 >= (rad - 1) * (rad - 1)))
                    setPixel(cx + xx, cy + yy, r, g, b);
            }
        }
    }

    void savePPM(const string& file) const {
        ofstream out(file, ios::binary);
        out << "P6\n" << W << ' ' << H << "\n255\n";
        out.write(reinterpret_cast<const char*>(pix.data()), (streamsize)pix.size());
    }
};

Point worldToPixel(const Point& p, int W, int H) {
    const double xmin = -10.0, xmax = 10.0;
    const double ymin = -7.0, ymax = 7.0;
    int x = (int)llround((p.x - xmin) / (xmax - xmin) * (W - 1));
    int y = (int)llround((ymax - p.y) / (ymax - ymin) * (H - 1));
    return {(double)x, (double)y};
}

using Stroke = vector<Point>;
vector<Stroke> makeLetter(char c);

vector<Point> sampleStrokes(const vector<Stroke>& strokes, int n) {
    vector<double> L(strokes.size(), 0.0);
    double total = 0.0;
    for (size_t s = 0; s < strokes.size(); ++s) {
        for (size_t k = 1; k < strokes[s].size(); ++k)
            L[s] += norm(strokes[s][k] - strokes[s][k - 1]);
        total += L[s];
    }

    vector<Point> ans;
    if (total == 0.0) return ans;

    for (int q = 0; q < n; ++q) {
        double target = (n == 1) ? 0.0 : (double)q / (n - 1) * total;
        double base = 0.0;
        size_t s = 0;
        while (s + 1 < strokes.size() && target > base + L[s]) {
            base += L[s];
            ++s;
        }
        double d = target - base;
        if (L[s] == 0.0) {
            ans.push_back(strokes[s].front());
            continue;
        }
        size_t k = 1;
        while (k < strokes[s].size()) {
            double seg = norm(strokes[s][k] - strokes[s][k - 1]);
            if (d <= seg || k + 1 == strokes[s].size()) {
                double t = (seg > 1e-12) ? d / seg : 0.0;
                t = clamp(t, 0.0, 1.0);
                ans.push_back(strokes[s][k - 1] + (strokes[s][k] - strokes[s][k - 1]) * t);
                break;
            }
            d -= seg;
            ++k;
        }
    }
    return ans;
}

vector<Stroke> makeLetter(char c) {

    auto S = [](initializer_list<Point> p) { return Stroke(p); };
    switch (c) {
        case 'P': return {
            S({{0.12,0.05},{0.12,0.95}}),
            S({{0.12,0.95},{0.67,0.95},{0.78,0.84},{0.78,0.66},{0.67,0.53},{0.12,0.53}})
        };
        case 'A': return {
            S({{0.10,0.05},{0.50,0.95},{0.90,0.05}}),
            S({{0.28,0.40},{0.72,0.40}})
        };
        case 'N': return {
            S({{0.10,0.05},{0.10,0.95},{0.90,0.05},{0.90,0.95}})
        };
        case 'I': return {
            S({{0.15,0.95},{0.85,0.95}}),
            S({{0.50,0.95},{0.50,0.05}}),
            S({{0.15,0.05},{0.85,0.05}})
        };
        case 'K': return {
            S({{0.12,0.05},{0.12,0.95}}),
            S({{0.12,0.50},{0.87,0.95}}),
            S({{0.12,0.50},{0.87,0.05}})
        };
        case 'R': return {
            S({{0.12,0.05},{0.12,0.95}}),
            S({{0.12,0.95},{0.65,0.95},{0.78,0.83},{0.78,0.65},{0.65,0.52},{0.12,0.52}}),
            S({{0.50,0.52},{0.88,0.05}})
        };
        default: return {S({{0,0},{1,1}})};
    }
}

vector<Point> targetForLetter(char c) {
    vector<Point> q = sampleStrokes(makeLetter(c), 20);

    double xmin = 1e9, xmax = -1e9, ymin = 1e9, ymax = -1e9;
    for (auto &p : q) {
        xmin = min(xmin, p.x); xmax = max(xmax, p.x);
        ymin = min(ymin, p.y); ymax = max(ymax, p.y);
    }
    double sx = 5.6 / max(1e-9, xmax - xmin);
    double sy = 7.0 / max(1e-9, ymax - ymin);
    double s = min(sx, sy);
    for (auto &p : q) {
        p.x = (p.x - 0.5 * (xmin + xmax)) * s;
        p.y = (p.y - 0.5 * (ymin + ymax)) * s;
    }
    return q;
}

bool connected(const vector<vector<int>>& adj) {
    int n = (int)adj.size();
    vector<int> vis(n, 0);
    queue<int> q;
    q.push(0); vis[0] = 1;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj[u]) if (!vis[v]) {
            vis[v] = 1; q.push(v);
        }
    }
    return accumulate(vis.begin(), vis.end(), 0) == n;
}

vector<Edge> erdosRenyiConnected(int N, double p, mt19937& rng, vector<vector<int>>& adj) {
    bernoulli_distribution bern(p);
    while (true) {
        adj.assign(N, {});
        vector<Edge> edges;
        for (int i = 0; i < N; ++i) {
            for (int j = i + 1; j < N; ++j) {
                if (bern(rng)) {
                    adj[i].push_back(j);
                    adj[j].push_back(i);
                    edges.push_back({i, j});
                }
            }
        }
        if (connected(adj)) return edges;
    }
}

int main() {
    const int N = 20;
    const double p = 0.25;
    const double dt = 0.03;
    const double kAnchor = 1.20;
    const double kFormation = 0.80;
    const int stepsPerLetter = 80;
    const int frameEvery = 2;
    const string NAME = "PANIKAR";

    mt19937 rng(3403);
    uniform_real_distribution<double> initDist(-9.0, 9.0);
    vector<Point> x(N);
    for (auto& p0 : x) p0 = {initDist(rng), initDist(rng)};

    vector<vector<int>> adj;
    vector<Edge> edges = erdosRenyiConnected(N, p, rng, adj);

    ofstream ge("results/P1_graph_edges.csv");
    ge << "u,v\n";
    for (auto e : edges) ge << e.u << ',' << e.v << '\n';

    ofstream tr("results/P1_trajectory.csv");
    tr << "letter,letter_index,step,agent,x,y,target_x,target_y\n";

    const int W = 900, H = 650;
    int frameId = 0;
    filesystem::create_directories("results/P1_frames");

    for (int li = 0; li < (int)NAME.size(); ++li) {
        char letter = NAME[li];
        vector<Point> r = targetForLetter(letter);

        for (int step = 0; step < stepsPerLetter; ++step) {
            vector<Point> u(N, {0,0});
            for (int i = 0; i < N; ++i) {
                Point anchor = r[i] - x[i];
                Point rel{0,0};
                if (!adj[i].empty()) {
                    for (int j : adj[i]) {
                        Point e = (r[i] - r[j]) - (x[i] - x[j]);
                        rel = rel + e;
                    }
                    rel = rel * (1.0 / adj[i].size());
                }
                u[i] = anchor * kAnchor + rel * kFormation;
            }
            for (int i = 0; i < N; ++i) x[i] = x[i] + u[i] * dt;

            for (int i = 0; i < N; ++i)
                tr << letter << ',' << li << ',' << step << ',' << i << ','
                   << x[i].x << ',' << x[i].y << ',' << r[i].x << ',' << r[i].y << '\n';

            if (step % frameEvery == 0) {
                Image img(W, H);

                for (auto e : edges) {
                    Point a = worldToPixel(x[e.u], W, H);
                    Point b = worldToPixel(x[e.v], W, H);
                    img.line((int)a.x, (int)a.y, (int)b.x, (int)b.y, 205, 205, 205);
                }

                for (auto &rp : r) {
                    Point a = worldToPixel(rp, W, H);
                    img.circle((int)a.x, (int)a.y, 5, 230, 90, 60, false);
                }

                for (int i = 0; i < N; ++i) {
                    Point a = worldToPixel(x[i], W, H);
                    img.circle((int)a.x, (int)a.y, 6, 30, 70, 210, true);
                }
                string fn = "results/P1_frames/frame_" + (ostringstream{} << setw(5) << setfill('0') << frameId).str() + ".ppm";
                img.savePPM(fn);
                ++frameId;
            }
        }
    }

    ofstream info("results/P1_info.txt");
    info << "Name: " << NAME << '\n';
    info << "N: " << N << '\n';
    info << "Erdos-Renyi p: " << p << '\n';
    info << "Number of edges: " << edges.size() << '\n';
    info << "dt: " << dt << '\n';
    info << "kAnchor: " << kAnchor << '\n';
    info << "kFormation: " << kFormation << '\n';
    info << "steps per letter: " << stepsPerLetter << '\n';
    info << "frames: " << frameId << '\n';

    cout << "Problem 1 complete. Connected graph with " << edges.size()
         << " edges; generated " << frameId << " PPM frames.\n";
    cout << "Create MP4 with: ffmpeg -framerate 20 -i results/P1_frames/frame_%05d.ppm -pix_fmt yuv420p P1_PANIKAR.mp4\n";
    return 0;
}
