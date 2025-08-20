from PyQt5.QtCore import QSize, QTimer, Qt
from PyQt5.QtWidgets import QApplication, QHBoxLayout, QMainWindow, QTableWidget, QTableWidgetItem, QVBoxLayout, QWidget, QLabel, QHeaderView, QAbstractItemView, QSizePolicy
import pyqtgraph as pg
import numpy as np
from pyqtgraph.Qt import QtCore, QtGui
import sys, queue, threading
from consumer import consumer_thread


class MainWindow(QMainWindow):
    
    def __init__(self, out_q: "queue.Queue[dict]", stop_evt: threading.Event):
        super().__init__()
        self.setStyleSheet("background-color: black")
        self.setWindowTitle("BTC L2 ORDERBOOK")
        self.setGeometry(100, 100, 900, 600)
        self.out_q = out_q
        self.stop_evt = stop_evt

        root = QWidget(); self.setCentralWidget(root)
        v = QVBoxLayout(root)
        v.setContentsMargins(8, 8, 8, 8)
        v.setSpacing(8)
        
          
        # spread/ mid 
        top = QHBoxLayout()
        self.label_spread = QLabel("spread: -")
        self.label_mid = QLabel("mid: -")
        
        for lab in (self.label_spread, self.label_mid):
            lab.setStyleSheet("color: white")
            
        top.addWidget(self.label_spread)
        top.addSpacing(16)
        top.addWidget(self.label_mid)
        top.addStretch()

        # best bid / ask 
        
        self.label_bb = QLabel("bb: -")
        self.label_ba = QLabel("ba: -")
        
        for lab in (self.label_bb, self.label_ba):
            lab.setStyleSheet("color: white")
        
        top.addWidget(self.label_bb)
        top.addSpacing(12)
        top.addWidget(self.label_ba)
        v.addLayout(top)
        


        # bid p/q ask p/q 
        
        h = QHBoxLayout()
        h.setSpacing(12)
        v.addLayout(h, 3)
        
        self.table_bids = QTableWidget(0, 2)
        self.table_bids.setHorizontalHeaderLabels(["Bid Price", "Bid Qty"])
        self.table_bids.verticalHeader().setVisible(False)
        self.table_bids.setStyleSheet("border: 1px solid black;")
        self.table_bids.setEditTriggers(self.table_bids.NoEditTriggers)
        
        self.table_asks = QTableWidget(0, 2)
        self.table_asks.setHorizontalHeaderLabels(["Ask Price", "Ask Qty"])
        self.table_asks.verticalHeader().setVisible(False)
        self.table_asks.setStyleSheet("border: 1px solid black;")
        self.table_asks.setEditTriggers(self.table_bids.NoEditTriggers)
        
        for tbl in (self.table_bids, self.table_asks):
            tbl.setSelectionMode(QAbstractItemView.NoSelection)
            tbl.setAlternatingRowColors(True)
            tbl.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
            tbl.setStyleSheet(
            """
            QTableWidget {
            background-color: #111;
            color: #eee;
            gridline-color: #444;
            alternate-background-color: #1a1a1a;  /* fixes white rows */
            }
            QHeaderView::section { background-color: #222; color: #ddd; }
            """
            )
            tbl.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)

        h.addWidget(self.table_bids, 1)
        h.addWidget(self.table_asks, 1)

        #heatmap 
        
        self.n_levels = 5
        self.hist_len = 200

        self.bid_hist = np.zeros((self.hist_len, self.n_levels), dtype=np.float32)
        self.ask_hist = np.zeros((self.hist_len, self.n_levels), dtype=np.float32)

        self.heat = pg.GraphicsLayoutWidget() 
        v.addWidget(self.heat, 1)
        
        #bids (heatmap)
   
        self.p_bid = self.heat.addPlot(row=0,col=0)
        self.p_bid.setTitle('<span style="color:#ddd">Bids </span>')
        self.p_bid.showAxis('bottom', False); self.p_bid.showAxis('left', False)
        self.p_bid.invertY(True)
        self.p_bid.setMenuEnabled(False)
        self.p_bid.getViewBox().setMouseEnabled(False, False)
        self.img_bid = pg.ImageItem()
        self.p_bid.addItem(self.img_bid)
     
        #asks (heatmap)
        self.p_ask = self.heat.addPlot(row=0,col=1)
        self.p_ask.setTitle('<span style="color:#ddd">Asks </span>')
        self.p_ask.showAxis('bottom', False); self.p_ask.showAxis('left', False)
        self.p_ask.invertY(True)
        self.p_ask.setMenuEnabled(False)
        self.p_ask.getViewBox().setMouseEnabled(False, False)
        self.img_ask = pg.ImageItem()
        self.p_ask.addItem(self.img_ask)
     
        
        self.heat.ci.layout.setColumnStretchFactor(0, 1)
        self.heat.ci.layout.setColumnStretchFactor(1, 1)

        self.p_bid.setAspectLocked(False)
        self.p_ask.setAspectLocked(False)

        try: 
            
            lut = pg.colormap.get('inferno').getLookupTable(0.0, 1.0, 256)

            try:
                from pyqtgraph import ColorBarItem
                cbar = ColorBarItem(colorMap=pg.colormap.get('inferno'),
                values=(0.0, 1.0), label='log(1+qty) / max')
                cbar.setImageItem(self.img_bid)
                self.heat.addItem(cbar, row=0, col=2)
                self.heat.ci.layout.setColumnStretchFactor(2, 0)

            except Exception:
                pass



        except Exception:
            lut = None 
        if lut is not None: 
            self.img_bid.setLookupTable(lut)
            self.img_ask.setLookupTable(lut)
            

        #axis/label for heatmap 
        
        for p in (self.p_bid, self.p_ask):
            p.showAxis('bottom', True)
            p.showAxis('left', True)
            p.showGrid(x=False, y=True, alpha=0.2)
            p.getViewBox().setMenuEnabled(False)
            p.getViewBox().setMouseEnabled(False, False)
            p.invertY(True)
            
        self.p_bid.setLabel('bottom', 'time', units='s ago')
        self.p_ask.setLabel('bottom', 'time', units='s ago')
        self.p_bid.setLabel('left',  'depth levels')
        self.p_ask.setLabel('left',  'depth levels')

        self.dt = 0.1                          
        self.total_window = self.hist_len * self.dt


        self.timer = QTimer(self)
        self.timer.timeout.connect(self.drain_queue)
        self.timer.start(50)


    def fill_order_table(self, table: QTableWidget, levels):
        
        table.setSortingEnabled(False)
        n = len(levels)
        if table.rowCount() != n:
            table.setRowCount(n)
       
        for r, lv in enumerate(levels):
            p = lv.get("price", "")
            q = lv.get("quantity", "")
            p_text = f"{p:.2f}" if isinstance(p, (int, float)) else str(p)
            q_text = f"{q:.8f}" if isinstance(q, (int, float)) else str(q)
            for c, text in enumerate((p_text, q_text)):
                item = table.item(r, c)
                if item is None: 
                    item = QTableWidgetItem(text)
                    item.setTextAlignment(Qt.AlignRight | Qt.AlignVCenter)
                    item.setFlags(Qt.ItemIsEnabled)
                    table.setItem(r, c, item)
                elif item.text() != text: 
                    item.setText(text)
    
               
        
               
    def drain_queue(self):
        
        while True:

        #filling tables / spread+ mid
            try:
                item = self.out_q.get_nowait() 
            except queue.Empty:
                break

            print("lens:", len(item.get("bids", [])), len(item.get("asks", [])),
                "top2 bids:", item.get("bids", [])[:2],
                "top2 asks:", item.get("asks", [])[:2],
                flush=True)


     
            s = item.get("spread"); m = item.get("mid")
            if s is not None: self.label_spread.setText(f"spread: {s:.4f}")
            if m is not None: self.label_mid.setText(f"mid: {m:.4f}")
        
            self.fill_order_table(self.table_bids, item.get("bids", []))
            self.fill_order_table(self.table_asks, item.get("asks", []))

        #filling heatmap + ba / bb
        
            b_lvls = item.get("bids", [])
            a_lvls = item.get("asks", [])
            
            if b_lvls:
                bb_p = b_lvls[0].get("price")
                bb_q = b_lvls[0].get("quantity")
                self.label_bb.setText(f"bb: {bb_p:.2f}")
            else:
                self.label_bb.setText("bb: -")

            if a_lvls:
                ba_p = a_lvls[0].get("price")
                ba_q = a_lvls[0].get("quantity")
                self.label_ba.setText(f"ba: {ba_p:.2f}")
            else:
                self.label_ba.setText("ba: -")
            
            b_quant = np.array([lv.get("quantity", 0.0) for lv in b_lvls], dtype=np.float32)
            a_quant = np.array([lv.get("quantity", 0.0) for lv in a_lvls], dtype=np.float32)
            
            def pad_levels(arr, n):
                if arr.size < n:
                    pad = np.full(n - arr.size, np.nan, dtype=np.float32)
                    return np.concatenate([arr, pad])
                return arr[:n]     
            
            b_quant = pad_levels(b_quant, self.n_levels)
            a_quant = pad_levels(a_quant, self.n_levels)

            b_row = np.log1p(b_quant)
            a_row = np.log1p(a_quant)

            eps = 1e-6
            b_den = np.nanmax(np.vstack([self.bid_hist, b_row[None, :]]), axis=0) + eps
            a_den = np.nanmax(np.vstack([self.ask_hist, a_row[None,:]]), axis=0) + eps
            b_row = b_row / b_den
            a_row = a_row / a_den
           
            self.bid_hist = np.roll(self.bid_hist, -1, axis=0); self.bid_hist[-1, :] = b_row
            self.ask_hist = np.roll(self.ask_hist, -1, axis=0); self.ask_hist[-1, :] = a_row
            
            self.img_bid.setImage(self.bid_hist.T, autoLevels=False)
            self.img_bid.resetTransform()
            self.img_bid.setRect(QtCore.QRectF(0, 0, self.hist_len, self.n_levels))
            self.img_ask.setImage(self.ask_hist.T, autoLevels=False)
            self.img_ask.resetTransform() 
            self.img_ask.setRect(QtCore.QRectF(0, 0, self.hist_len, self.n_levels))

            T = self.total_window
            rect = QtCore.QRectF(-T, 0, T, self.n_levels)
            for img in (self.img_bid, self.img_ask):
                img.resetTransform()
                img.setRect(rect)

            tick_step = max(1, int(T // 5)) 
            ticks = [(-s, f"{s}s") for s in range(0, int(T) + 1, tick_step)]
            self.p_bid.getAxis('bottom').setTicks([ticks])
            self.p_ask.getAxis('bottom').setTicks([ticks])
            
            lvl_ticks = [(i + 0.5, f"L{i+1}") for i in range(self.n_levels)]
            self.p_bid.getAxis('left').setTicks([lvl_ticks])
            self.p_ask.getAxis('left').setTicks([lvl_ticks])

            mx = max(np.nanmax(self.bid_hist), np.nanmax(self.ask_hist), 1e-9)
            self.img_bid.setLevels((0.0, float(mx)))
            self.img_ask.setLevels((0.0, float(mx)))
            

    def closeEvent(self, e):
        self.stop_evt.set()
        super().closeEvent(e)
        



if __name__ == "__main__":
    
    q = queue.Queue(maxsize=5)
   
    stop_evt = threading.Event() 
    t = threading.Thread(target=consumer_thread, args = (q, stop_evt), daemon=True)
    t.start() 
   
    app = QApplication(sys.argv)
    window = MainWindow(q, stop_evt) 
    window.show() 
    app.exec() 
