  0:     LD  4,0(0)		Set GP
  3:     ST  0,0(0)		Clear dMem[0]
  4:     ST  5,0(6)		Save FP
  5:    LDA  6,-1(6)		
  7:    LDA  0,1(7)		Get PC for Halt
  9:   HALT  0,0,0		Halt
 10:     ST  0,-1(5)		Input Function, store return addr
 11:     IN  0,0,0		     Get input
 12:     LD  7,-1(5)		     Return (end of function)
 13:     ST  0,-1(5)		Output Function, store return addr
 14:     LD  0,-2(5)		     Get output
 15:    OUT  0,0,0		     Give output
 16:     LD  7,-1(5)		     Return (end of function)
 17:     ST  0,-1(5)		Function: store return address in dMem[FP-1]
 18:     LD  0,-2(5)		  variable: AC0 = value of variable
 19:     ST  0,0(6)		
 20:    LDA  6,-1(6)		
 21:    LDC  0,30(0)		
 22:    LDA  6,1(6)		
 23:     LD  1,0(6)		
 24:    SUB  0,1,0		
 26:    LDC  0,0(0)		
 27:    LDC  1,0(0)		
 29:    LDC  0,1(0)		
 25:    JGE  0,3(7)	29	
 28:    JEQ  1,1(7)	30	
 31:     LD  7,-1(5)		Returning (end of function)
 30:    JEQ  0,1(7)	32	  if test: Jump to end if false (exp == 0)
 32:     LD  0,-2(5)		  variable: AC0 = value of variable
 33:     ST  0,0(6)		
 34:    LDA  6,-1(6)		
 35:    LDC  0,0(0)		
 36:    LDA  6,1(6)		
 37:     LD  1,0(6)		
 38:    SUB  0,1,0		
 40:    LDC  0,0(0)		
 41:    LDC  1,0(0)		
 43:    LDC  0,1(0)		
 39:    JEQ  0,3(7)	43	
 42:    JEQ  1,1(7)	44	
 45:    LDC  0,0(0)		
 46:     ST  0,0(6)		  assignment: save value
 47:    LDA  6,-1(6)		
 48:     LD  0,-2(5)		  variable: AC0 = value of variable
 50:   HALT  0,0,0		  variable: Stop. Neg subscripts illegal.
 49:    JGE  0,1(7)	51	  variable: Jump if subscript >= 0
 51:    LDA  1,0(4)		
 52:    SUB  1,1,0		
 53:    LDA  6,1(6)		  assignment: retrieve value
 54:     LD  0,0(6)		
 55:     ST  0,0(1)		  assignment: variable = dMem[ac1] = value
 44:    JEQ  0,11(7)	56	  if test: Jump to end if false (exp == 0)
 56:     LD  0,-2(5)		  variable: AC0 = value of variable
 57:     ST  0,0(6)		
 58:    LDA  6,-1(6)		
 59:    LDC  0,1(0)		
 60:    LDA  6,1(6)		
 61:     LD  1,0(6)		
 62:    SUB  0,1,0		
 64:    LDC  0,0(0)		
 65:    LDC  1,0(0)		
 67:    LDC  0,1(0)		
 63:    JEQ  0,3(7)	67	
 66:    JEQ  1,1(7)	68	
 69:    LDC  0,1(0)		
 70:     ST  0,0(6)		  assignment: save value
 71:    LDA  6,-1(6)		
 72:     LD  0,-2(5)		  variable: AC0 = value of variable
 74:   HALT  0,0,0		  variable: Stop. Neg subscripts illegal.
 73:    JGE  0,1(7)	75	  variable: Jump if subscript >= 0
 75:    LDA  1,0(4)		
 76:    SUB  1,1,0		
 77:    LDA  6,1(6)		  assignment: retrieve value
 78:     LD  0,0(6)		
 79:     ST  0,0(1)		  assignment: variable = dMem[ac1] = value
 68:    JEQ  0,11(7)	80	  if test: Jump to end if false (exp == 0)
 80:     LD  0,-2(5)		  variable: AC0 = value of variable
 81:     ST  0,0(6)		
 82:    LDA  6,-1(6)		
 83:    LDC  0,1(0)		
 84:    LDA  6,1(6)		
 85:     LD  1,0(6)		
 86:    SUB  0,1,0		
 88:    LDC  0,0(0)		
 89:    LDC  1,0(0)		
 91:    LDC  0,1(0)		
 87:    JGT  0,3(7)	91	
 90:    JEQ  1,1(7)	92	
 93:    LDC  0,-1(0)		
 94:     ST  0,0(6)		  assignment: save value
 95:    LDA  6,-1(6)		
 96:     LD  0,-2(5)		  variable: AC0 = value of variable
 98:   HALT  0,0,0		  variable: Stop. Neg subscripts illegal.
 97:    JGE  0,1(7)	99	  variable: Jump if subscript >= 0
 99:    LDA  1,0(4)		
100:    SUB  1,1,0		
101:    LDA  6,1(6)		  assignment: retrieve value
102:     LD  0,0(6)		
103:     ST  0,0(1)		  assignment: variable = dMem[ac1] = value
 92:    JEQ  0,11(7)	104	  if test: Jump to end if false (exp == 0)
104:     ST  5,0(6)		     Function call, save old FP
105:    LDA  6,-1(6)		
106:    LDA  6,-1(6)		     Save space for return addr
107:     LD  0,-2(5)		  variable: AC0 = value of variable
108:     ST  0,0(6)		
109:    LDA  6,-1(6)		
110:    LDC  0,1(0)		
111:    LDA  6,1(6)		
112:     LD  1,0(6)		
113:    ADD  0,1,0		
114:     ST  0,0(6)		
115:    LDA  6,-1(6)		
116:    LDA  5,3(6)		     Set FP to top of frame
117:    LDA  6,0(6)		     Set SP after locals
118:    LDA  0,1(7)		     Get return addr
119:    LDA  7,-103(7)	17	     Jump to function
120:    LDA  6,0(5)		     Restore old SP
121:     LD  5,0(5)		     Restore old FP
122:     LD  7,-1(5)		End of function (set PC = return addr)
123:     ST  0,-1(5)		Function: store return address in dMem[FP-1]
124:     LD  0,-2(5)		  variable: AC0 = value of variable
126:   HALT  0,0,0		  variable: Stop. Neg subscripts illegal.
125:    JGE  0,1(7)	127	  variable: Jump if subscript >= 0
127:    LDA  1,0(4)		
128:    SUB  1,1,0		
129:     LD  0,0(1)		
130:     ST  0,0(6)		
131:    LDA  6,-1(6)		
132:    LDC  0,-1(0)		
133:    LDA  6,1(6)		
134:     LD  1,0(6)		
135:    SUB  0,1,0		
137:    LDC  0,0(0)		
138:    LDC  1,0(0)		
140:    LDC  0,1(0)		
136:    JGT  0,3(7)	140	
139:    JEQ  1,1(7)	141	
142:     LD  0,-2(5)		  variable: AC0 = value of variable
144:   HALT  0,0,0		  variable: Stop. Neg subscripts illegal.
143:    JGE  0,1(7)	145	  variable: Jump if subscript >= 0
145:    LDA  1,0(4)		
146:    SUB  1,1,0		
147:     LD  0,0(1)		
148:     LD  7,-1(5)		Returning (end of function)
141:    JEQ  0,7(7)	149	  if test: Jump to end if false (exp == 0)
149:     ST  5,0(6)		     Function call, save old FP
150:    LDA  6,-1(6)		
151:    LDA  6,-1(6)		     Save space for return addr
152:     LD  0,-2(5)		  variable: AC0 = value of variable
153:     ST  0,0(6)		
154:    LDA  6,-1(6)		
155:    LDC  0,1(0)		
156:    LDA  6,1(6)		
157:     LD  1,0(6)		
158:    SUB  0,1,0		
159:     ST  0,0(6)		
160:    LDA  6,-1(6)		
161:    LDA  5,3(6)		     Set FP to top of frame
162:    LDA  6,0(6)		     Set SP after locals
163:    LDA  0,1(7)		     Get return addr
164:    LDA  7,-42(7)	123	     Jump to function
165:    LDA  6,0(5)		     Restore old SP
166:     LD  5,0(5)		     Restore old FP
167:     ST  0,0(6)		
168:    LDA  6,-1(6)		
169:     ST  5,0(6)		     Function call, save old FP
170:    LDA  6,-1(6)		
171:    LDA  6,-1(6)		     Save space for return addr
172:     LD  0,-2(5)		  variable: AC0 = value of variable
173:     ST  0,0(6)		
174:    LDA  6,-1(6)		
175:    LDC  0,2(0)		
176:    LDA  6,1(6)		
177:     LD  1,0(6)		
178:    SUB  0,1,0		
179:     ST  0,0(6)		
180:    LDA  6,-1(6)		
181:    LDA  5,3(6)		     Set FP to top of frame
182:    LDA  6,0(6)		     Set SP after locals
183:    LDA  0,1(7)		     Get return addr
184:    LDA  7,-62(7)	123	     Jump to function
185:    LDA  6,0(5)		     Restore old SP
186:     LD  5,0(5)		     Restore old FP
187:    LDA  6,1(6)		
188:     LD  1,0(6)		
189:    ADD  0,1,0		
190:     ST  0,0(6)		  assignment: save value
191:    LDA  6,-1(6)		
192:     LD  0,-2(5)		  variable: AC0 = value of variable
194:   HALT  0,0,0		  variable: Stop. Neg subscripts illegal.
193:    JGE  0,1(7)	195	  variable: Jump if subscript >= 0
195:    LDA  1,0(4)		
196:    SUB  1,1,0		
197:    LDA  6,1(6)		  assignment: retrieve value
198:     LD  0,0(6)		
199:     ST  0,0(1)		  assignment: variable = dMem[ac1] = value
200:     LD  0,-2(5)		  variable: AC0 = value of variable
202:   HALT  0,0,0		  variable: Stop. Neg subscripts illegal.
201:    JGE  0,1(7)	203	  variable: Jump if subscript >= 0
203:    LDA  1,0(4)		
204:    SUB  1,1,0		
205:     LD  0,0(1)		
206:     LD  7,-1(5)		Returning (end of function)
207:     LD  7,-1(5)		End of function (set PC = return addr)
208:     ST  0,-1(5)		Function: store return address in dMem[FP-1]
209:     LD  0,-2(5)		  variable: AC0 = value of variable
210:     ST  0,0(6)		
211:    LDA  6,-1(6)		
212:    LDC  0,30(0)		
213:    LDA  6,1(6)		
214:     LD  1,0(6)		
215:    SUB  0,1,0		
217:    LDC  0,0(0)		
218:    LDC  1,0(0)		
220:    LDC  0,1(0)		
216:    JGE  0,3(7)	220	
219:    JEQ  1,1(7)	221	
222:    LDC  0,-1(0)		
223:     LD  7,-1(5)		Returning (end of function)
221:    JEQ  0,2(7)	224	  if test: Jump to end if false (exp == 0)
224:     ST  5,0(6)		     Function call, save old FP
225:    LDA  6,-1(6)		
226:    LDA  6,-1(6)		     Save space for return addr
227:     ST  5,0(6)		     Function call, save old FP
228:    LDA  6,-1(6)		
229:    LDA  6,-1(6)		     Save space for return addr
230:     LD  0,-2(5)		  variable: AC0 = value of variable
231:     ST  0,0(6)		
232:    LDA  6,-1(6)		
233:    LDA  5,3(6)		     Set FP to top of frame
234:    LDA  6,0(6)		     Set SP after locals
235:    LDA  0,1(7)		     Get return addr
236:    LDA  7,-114(7)	123	     Jump to function
237:    LDA  6,0(5)		     Restore old SP
238:     LD  5,0(5)		     Restore old FP
239:     ST  0,0(6)		
240:    LDA  6,-1(6)		
241:    LDA  5,3(6)		     Set FP to top of frame
242:    LDA  6,0(6)		     Set SP after locals
243:    LDA  0,1(7)		     Get return addr
244:    LDA  7,-232(7)	13	     Jump to function
245:    LDA  6,0(5)		     Restore old SP
246:     LD  5,0(5)		     Restore old FP
247:     ST  5,0(6)		     Function call, save old FP
248:    LDA  6,-1(6)		
249:    LDA  6,-1(6)		     Save space for return addr
250:     LD  0,-2(5)		  variable: AC0 = value of variable
251:     ST  0,0(6)		
252:    LDA  6,-1(6)		
253:    LDC  0,1(0)		
254:    LDA  6,1(6)		
255:     LD  1,0(6)		
256:    ADD  0,1,0		
257:     ST  0,0(6)		
258:    LDA  6,-1(6)		
259:    LDA  5,3(6)		     Set FP to top of frame
260:    LDA  6,0(6)		     Set SP after locals
261:    LDA  0,1(7)		     Get return addr
262:    LDA  7,-55(7)	208	     Jump to function
263:    LDA  6,0(5)		     Restore old SP
264:     LD  5,0(5)		     Restore old FP
265:    LDC  0,0(0)		
266:     LD  7,-1(5)		Returning (end of function)
267:     LD  7,-1(5)		End of function (set PC = return addr)
268:     ST  0,-1(5)		Function: store return address in dMem[FP-1]
269:     ST  5,0(6)		     Function call, save old FP
270:    LDA  6,-1(6)		
271:    LDA  6,-1(6)		     Save space for return addr
272:    LDC  0,0(0)		
273:     ST  0,0(6)		
274:    LDA  6,-1(6)		
275:    LDA  5,3(6)		     Set FP to top of frame
276:    LDA  6,0(6)		     Set SP after locals
277:    LDA  0,1(7)		     Get return addr
278:    LDA  7,-262(7)	17	     Jump to function
279:    LDA  6,0(5)		     Restore old SP
280:     LD  5,0(5)		     Restore old FP
281:     ST  5,0(6)		     Function call, save old FP
282:    LDA  6,-1(6)		
283:    LDA  6,-1(6)		     Save space for return addr
284:    LDC  0,0(0)		
285:     ST  0,0(6)		
286:    LDA  6,-1(6)		
287:    LDA  5,3(6)		     Set FP to top of frame
288:    LDA  6,0(6)		     Set SP after locals
289:    LDA  0,1(7)		     Get return addr
290:    LDA  7,-83(7)	208	     Jump to function
291:    LDA  6,0(5)		     Restore old SP
292:     LD  5,0(5)		     Restore old FP
293:     LD  7,-1(5)		End of function (set PC = return addr)
  1:    LDA  5,-30(4)		Set FP below globals
  2:    LDA  6,-30(4)		Set SP below globals
  8:    LDA  7,259(7)	268	Jump to main
  6:    LDA  6,-1(6)		Set SP below locals
