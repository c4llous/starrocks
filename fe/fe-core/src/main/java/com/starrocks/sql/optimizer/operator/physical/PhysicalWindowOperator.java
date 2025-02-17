// This file is licensed under the Elastic License 2.0. Copyright 2021 StarRocks Limited.
package com.starrocks.sql.optimizer.operator.physical;

import com.starrocks.analysis.AnalyticWindow;
import com.starrocks.sql.optimizer.OptExpression;
import com.starrocks.sql.optimizer.OptExpressionVisitor;
import com.starrocks.sql.optimizer.base.ColumnRefSet;
import com.starrocks.sql.optimizer.base.Ordering;
import com.starrocks.sql.optimizer.operator.OperatorVisitor;
import com.starrocks.sql.optimizer.operator.Projection;
import com.starrocks.sql.optimizer.operator.scalar.CallOperator;
import com.starrocks.sql.optimizer.operator.scalar.ColumnRefOperator;
import com.starrocks.sql.optimizer.operator.scalar.ScalarOperator;

import java.util.List;
import java.util.Map;
import java.util.Objects;

import static com.starrocks.sql.optimizer.operator.OperatorType.PHYSICAL_WINDOW;

public class PhysicalWindowOperator extends PhysicalOperator {
    private final Map<ColumnRefOperator, CallOperator> analyticCall;
    private final List<ScalarOperator> partitionExpressions;
    private final List<Ordering> orderByElements;
    private final AnalyticWindow analyticWindow;
    private final List<Ordering> enforceOrderBy;

    public PhysicalWindowOperator(Map<ColumnRefOperator, CallOperator> analyticCall,
                                  List<ScalarOperator> partitionExpressions,
                                  List<Ordering> orderByElements,
                                  AnalyticWindow analyticWindow,
                                  List<Ordering> enforceOrderBy,
                                  long limit,
                                  ScalarOperator predicate,
                                  Projection projection) {
        super(PHYSICAL_WINDOW);
        this.analyticCall = analyticCall;
        this.partitionExpressions = partitionExpressions;
        this.orderByElements = orderByElements;
        this.analyticWindow = analyticWindow;
        this.enforceOrderBy = enforceOrderBy;

        this.limit = limit;
        this.predicate = predicate;
        this.projection = projection;
    }

    public Map<ColumnRefOperator, CallOperator> getAnalyticCall() {
        return analyticCall;
    }

    public List<ScalarOperator> getPartitionExpressions() {
        return partitionExpressions;
    }

    public List<Ordering> getOrderByElements() {
        return orderByElements;
    }

    public AnalyticWindow getAnalyticWindow() {
        return analyticWindow;
    }

    public List<Ordering> getEnforceOrderBy() {
        return enforceOrderBy;
    }

    @Override
    public <R, C> R accept(OperatorVisitor<R, C> visitor, C context) {
        return visitor.visitPhysicalAnalytic(this, context);
    }

    @Override
    public <R, C> R accept(OptExpressionVisitor<R, C> visitor, OptExpression optExpression, C context) {
        return visitor.visitPhysicalAnalytic(optExpression, context);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null || getClass() != o.getClass()) {
            return false;
        }
        PhysicalWindowOperator that = (PhysicalWindowOperator) o;
        return Objects.equals(analyticCall, that.analyticCall) &&
                Objects.equals(partitionExpressions, that.partitionExpressions) &&
                Objects.equals(orderByElements, that.orderByElements) &&
                Objects.equals(analyticWindow, that.analyticWindow);
    }

    @Override
    public int hashCode() {
        return Objects.hash(analyticCall, partitionExpressions, orderByElements, analyticWindow);
    }

    @Override
    public ColumnRefSet getUsedColumns() {
        ColumnRefSet set = super.getUsedColumns();
        analyticCall.values().forEach(d -> set.union(d.getUsedColumns()));
        partitionExpressions.forEach(d -> set.union(d.getUsedColumns()));
        orderByElements.forEach(o -> set.union(o.getColumnRef()));
        return set;
    }
}
