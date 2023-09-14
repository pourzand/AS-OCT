<!DOCTYPE html>
<html>

<head>
<style>
table.list {
	border-collapse: collapse;
}
.list {
    border: 1px solid black;
	padding: 10px;

}
}

.equ {

	border: 0px;
	padding: 10px;
	vertical-align: top;
}
th {
  text-align: left;
}

table#t01 {
	width:100%;
}

table#t01 tr:nth-child(even) {
  background-color: #eee;
}
table#t01 tr:nth-child(odd) {
 background-color: #fff;
}

table#t01 th {
 background-color: #fff;
}


table#t02 tr:nth-child(even) {
  background-color: #fff;
}
table#t02 tr:nth-child(odd) {
 background-color: #fff;
}

</style>
</head>

<body>

<h1>
{{ gene }} <br>
{{ gene_binary }}
</h1>

<h3>
Fitness: {{ final_result["fitness"] }}
</h3>

<img width=100% src=" to_html(final_result["error_hist_plot"]) " >


<h1>
Overall Performance Summary
</h1>

<table class="equ" id="t01">
<!-- 	<col width="200">
	<col width="175">
	<col width="175">
	<col width="175">
	<col width="175"> -->
	<tr class="equ">
		<th class="equ" width=120px> </th>
		<th class="equ">Carina</th>
		<th class="equ">GE Junction</th>
		<th class="equ">ETT Tip</th>
		<th class="equ">NG Tube</th>
	</tr>

	<tr class="equ">
		<th class="equ">[x error]</th>
		<td class="equ"></td>
	</tr>
	<tr class="equ">
		<th class="equ">mean</th>
		<td class="equ">{{ final_result["Crina"].get("x_err_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("x_err_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("x_err_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("x_err_mean", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">std</th>
		<td class="equ">{{ final_result["Crina"].get("x_err_std", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("x_err_std", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("x_err_std", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("x_err_std", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">median</th>
		<td class="equ">{{ final_result["Crina"].get("x_err_median", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("x_err_median", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("x_err_median", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("x_err_median", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">1st/3rd quartiles</th>
		<td class="equ">{{ final_result["Crina"].get("x_err_q1", "N/A") }}, {{ final_result["Crina"].get("x_err_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("x_err_q1", "N/A") }}, {{ final_result["GEjct"].get("x_err_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("x_err_q1", "N/A") }}, {{ final_result["EtTip"].get("x_err_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("x_err_q1", "N/A") }}, {{ final_result["NgTub"].get("x_err_q3", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">[y error]</th>
		<td class="equ"></td>
	</tr>
	<tr class="equ">
		<th class="equ">mean</th>
		<td class="equ">{{ final_result["Crina"].get("y_err_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("y_err_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("y_err_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("y_err_mean", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">std</th>
		<td class="equ">{{ final_result["Crina"].get("y_err_std", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("y_err_std", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("y_err_std", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("y_err_std", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">median</th>
		<td class="equ">{{ final_result["Crina"].get("y_err_median", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("y_err_median", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("y_err_median", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("y_err_median", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">1st/3rd quartiles</th>
		<td class="equ">{{ final_result["Crina"].get("y_err_q1", "N/A") }}, {{ final_result["Crina"].get("y_err_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("y_err_q1", "N/A") }}, {{ final_result["GEjct"].get("y_err_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("y_err_q1", "N/A") }}, {{ final_result["EtTip"].get("y_err_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("y_err_q1", "N/A") }}, {{ final_result["NgTub"].get("y_err_q3", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">[total error]</th>
		<td class="equ"></td>
	</tr>
	<tr class="equ">
		<th class="equ">mean</th>
		<td class="equ">{{ final_result["Crina"].get("tot_err_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("tot_err_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("tot_err_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("tot_err_mean", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">std</th>
		<td class="equ">{{ final_result["Crina"].get("tot_err_std", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("tot_err_std", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("tot_err_std", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("tot_err_std", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">median</th>
		<td class="equ">{{ final_result["Crina"].get("tot_err_median", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("tot_err_median", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("tot_err_median", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("tot_err_median", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">1st/3rd quartiles</th>
		<td class="equ">{{ final_result["Crina"].get("tot_err_q1", "N/A") }}, {{ final_result["Crina"].get("tot_err_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("tot_err_q1", "N/A") }}, {{ final_result["GEjct"].get("tot_err_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("tot_err_q1", "N/A") }}, {{ final_result["EtTip"].get("tot_err_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("tot_err_q1", "N/A") }}, {{ final_result["NgTub"].get("tot_err_q3", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">[dice coeff]</th>
		<td class="equ"></td>
	</tr>
	<tr class="equ">
		<th class="equ">mean</th>
		<td class="equ">{{ final_result["Crina"].get("dice_coefficient_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("dice_coefficient_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("dice_coefficient_mean", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("dice_coefficient_mean", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">std</th>
		<td class="equ">{{ final_result["Crina"].get("dice_coefficient_std", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("dice_coefficient_std", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("dice_coefficient_std", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("dice_coefficient_std", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">median</th>
		<td class="equ">{{ final_result["Crina"].get("dice_coefficient_median", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("dice_coefficient_median", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("dice_coefficient_median", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("dice_coefficient_median", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">1st/3rd quartiles</th>
		<td class="equ">{{ final_result["Crina"].get("dice_coefficient_q1", "N/A") }}, {{ final_result["Crina"].get("dice_coefficient_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("dice_coefficient_q1", "N/A") }}, {{ final_result["GEjct"].get("dice_coefficient_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("dice_coefficient_q1", "N/A") }}, {{ final_result["EtTip"].get("dice_coefficient_q3", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("dice_coefficient_q1", "N/A") }}, {{ final_result["NgTub"].get("dice_coefficient_q3", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Number annotated cases:</th>
		<td class="equ">{{ final_result["Crina"].get("num_annotated", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("num_annotated", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("num_annotated", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("num_annotated", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Number good predictions:</th>
		<td class="equ">{{ final_result["Crina"]["num_good_markings"] }} cases with meeting {{ final_result["Crina"]["threshold"] }} threshold</td>
		<td class="equ">{{ final_result["GEjct"]["num_good_markings"] }} cases with meeting {{ final_result["GEjct"]["threshold"] }} threshold</td>
		<td class="equ">{{ final_result["EtTip"]["num_good_markings"] }} cases with meeting {{ final_result["EtTip"]["threshold"] }} threshold</td>
		<td class="equ">{{ final_result["NgTub"]["num_good_markings"] }} cases with meeting {{ final_result["NgTub"]["threshold"] }} threshold</td>
	</tr>
	<!-- <tr class="equ">
		<td class="equ">Bad markings:</td>
		<td class="equ">{{ final_result["Crina"]["num_bad_markings"] }}</td>
	</tr> -->
	<tr class="equ">
		<th class="equ">Detection:</th>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["Crina"].get("tp", "N/A") }}</td>
					<td class="list">FP: {{ final_result["Crina"].get("fp", "N/A") }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["Crina"].get("fn", "N/A") }}</td>
					<td class="list">TN: {{ final_result["Crina"].get("tn", "N/A") }}</td>
				</tr>
			</table>

		</td>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["GEjct"].get("tp", "N/A") }}</td>
					<td class="list">FP: {{ final_result["GEjct"].get("fp", "N/A") }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["GEjct"].get("fn", "N/A") }}</td>
					<td class="list">TN: {{ final_result["GEjct"].get("tn", "N/A") }}</td>
				</tr>
			</table>

		</td>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["EtTip"].get("tp", "N/A") }}</td>
					<td class="list">FP: {{ final_result["EtTip"].get("fp", "N/A") }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["EtTip"].get("fn", "N/A") }}</td>
					<td class="list">TN: {{ final_result["EtTip"].get("tn", "N/A") }}</td>
				</tr>
			</table>

		</td>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["NgTub"].get("tp", "N/A") }}</td>
					<td class="list">FP: {{ final_result["NgTub"].get("fp", "N/A") }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["NgTub"].get("fn", "N/A") }}</td>
					<td class="list">TN: {{ final_result["NgTub"].get("tn", "N/A") }}</td>
				</tr>
			</table>

		</td>
	</tr>

	<tr class="equ">
		<th class="equ">Sensitivity:</th>
		<td class="equ">{{ final_result["Crina"].get("sensitivity", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("sensitivity", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("sensitivity", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("sensitivity", "N/A") }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Specificity:</th>
		<td class="equ">{{ final_result["Crina"].get("specificity", "N/A") }}</td>
		<td class="equ">{{ final_result["GEjct"].get("specificity", "N/A") }}</td>
		<td class="equ">{{ final_result["EtTip"].get("specificity", "N/A") }}</td>
		<td class="equ">{{ final_result["NgTub"].get("specificity", "N/A") }}</td>
	</tr>

</table>

<!-- 
# - ref present/absent
# - MIU output present/absent
# - TP/FP/FN/TN
# - DCE Score
# - DCE -->


{% if final_result["bad_cases"]["Crina"] -%}
<h1>High error cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Carina Error</th>
		<th class="list">GE Junction Error</th>
		<th class="list">ETT Tip Error</th>
		<th class="list">NG Tube Error</th>
		<th class="list">ET Correct</th>
		<th class="list">NG Correct</th>
		<!-- <th class="list">x error</th>
		<th class="list">y error</th>
		<th class="list">total error</th> -->
		<!-- <th class="list">Detection</th> -->
	</tr>
	{% for key in final_result["bad_cases"]["Crina"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Crina"].get("tot_err")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["GEjct"].get("tot_err")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("tot_err")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("dice_coefficient")}}</td>
		<td class="list"></td>
		<td class="list"></td>
	</tr>
{%- endfor %}
</table>
{%- endif %}


<!-- {% if final_result["misplaced"] -%}
<h1>Incorrect placement cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Carina Error</th>
		<th class="list">GE Junction Error</th>
		<th class="list">ETT Tip Error</th>
		<th class="list">NG Tube Error</th>
		<th class="list">ET Correct</th>
		<th class="list">NG Correct</th>
	</tr>
{% for key in final_result["misplaced"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Crina"].get("tot_err", final_result["per_case_result"][key]["Crina"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["GEjct"].get("tot_err", final_result["per_case_result"][key]["GEjct"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("tot_err", final_result["per_case_result"][key]["EtTip"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("tot_err", final_result["per_case_result"][key]["NgTub"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("correct")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("correct")}}</td>
	</tr>
{%- endfor %}
</table>
{%- endif %} -->


{% if final_result["good_cases"]["Crina"] -%}
<h1>Other cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Carina Error</th>
		<th class="list">GE Junction Error</th>
		<th class="list">ETT Tip Error</th>
		<th class="list">NG Tube Error</th>
		<th class="list">ET Correct</th>
		<th class="list">NG Correct</th>
	</tr>
{% for key in final_result["good_cases"]["Crina"] -%}
<tr class="list">
	<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
	<td class="list">{{final_result["per_case_result"][key]["Crina"].get("tot_err")}}</td>
	<td class="list">{{final_result["per_case_result"][key]["GEjct"].get("tot_err")}}</td>
	<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("tot_err")}}</td>
	<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("dice_coefficient")}}</td>
	<td class="list"></td>
	<td class="list"></td>
</tr>
{%- endfor %}
</table>


{% else -%}
<h1>All cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Carina Error</th>
		<th class="list">GE Junction Error</th>
		<th class="list">ETT Tip Error</th>
		<th class="list">NG Tube Error</th>
		<th class="list">ET Correct</th>
		<th class="list">NG Correct</th>
	</tr>
{% for key in keys -%}
<tr class="list">
	<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
	<td class="list">{{final_result["per_case_result"][key]["Crina"].get("tot_err")}}</td>
	<td class="list">{{final_result["per_case_result"][key]["GEjct"].get("tot_err")}}</td>
	<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("tot_err")}}</td>
	<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("dice_coefficient")}}</td>
	<td class="list"></td>
	<td class="list"></td>
</tr>
{%- endfor %}
</table>
{%- endif %}


</body>

</html>